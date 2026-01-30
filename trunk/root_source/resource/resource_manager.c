/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "resource_manager.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep
#include "external/md5/md5.h"

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_RES_DEBUG_LOG        (0) 

#if IS_SUPPORT_RES_DEBUG_LOG
#define LOG_RES_DEBUG(format, args...)   LOG_RES(format, ##args)
#else
#define LOG_RES_DEBUG(format, args...)
#endif  

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static resource_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_LVGL
/// 检查资源包版本是否兼容
static bool img_res_version_compatible(uint16_t ver)
{
    const uint16_t cur = (uint16_t)IMG_RES_VERSION_CURRENT;
    const uint16_t cur_major = cur / 100;
    const uint16_t cur_minor = cur % 100;

    const uint16_t ver_major_new = ver / 100;
    const uint16_t ver_minor_new = ver % 100;

    const uint16_t ver_major_legacy = ver >> 8;
    const uint16_t ver_minor_legacy = ver & 0xFF;

    LOG_RES_DEBUG("img_res_version_compatible: ver=%u(cur=%u) new=%u.%02u legacy=%u.%02u cur=%u.%02u",
            (unsigned)ver, (unsigned)cur,
            (unsigned)ver_major_new, (unsigned)ver_minor_new,
            (unsigned)ver_major_legacy, (unsigned)ver_minor_legacy,
            (unsigned)cur_major, (unsigned)cur_minor);

    // New encoding: major matches
    if (ver_major_new == cur_major) {
        return true;
    }

    // Legacy encoding: accept only if major+minor exactly match current
    if (ver_major_legacy == cur_major && ver_minor_legacy == cur_minor) {
        return true;
    }

    return false;
}

/// 检查魔数是否有效
static bool img_res_magic_valid(uint32_t magic)
{
    return (magic == IMG_RES_MAGIC);
}

/// 计算最大可容纳资源数量
static uint16_t img_res_get_max_count(void)
{
    return (uint16_t)((IMG_RES_HEADER_FIXED_SIZE - sizeof(img_res_head_wrapper_t)) / sizeof(img_res_obj_info_t));
}

/// 计算头部结构实际占用大小
static uint32_t img_res_calc_header_size(uint32_t res_count)
{
    return (uint32_t)sizeof(img_res_head_wrapper_t) + res_count * (uint32_t)sizeof(img_res_obj_info_t);
}

static int img_res_flash_read(uint32_t addr, void *buf, uint32_t size)
{
    return platform_flash_read(PARTITION_IMG_INT_DATA_NAME, addr, (uint8_t *)buf, size);
}

static int img_res_flash_read_by_partition(const char *partition, uint32_t addr, void *buf, uint32_t size)
{
    if (!partition || !buf || size == 0) {
        return -1;
    }
    return platform_flash_read(partition, addr, (uint8_t *)buf, size);
}

static const char *img_res_get_partition_name(void)
{
#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_INTERNAL_FLASH
    return PARTITION_IMG_INT_DATA_NAME;
#else
    return PARTITION_IMG_EXT_DATA_NAME;
#endif
}

static bool img_res_get_base_addr(uint32_t *base_addr)
{
    EXCEPTION_CHECK_BOOL(base_addr);
#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_INTERNAL_FLASH
    *base_addr = (uint32_t)IMG_INT_DATA_START_ADDR;
#else
    *base_addr = (uint32_t)IMG_EXT_DATA_START_ADDR;
#endif
    return true;
}

static int img_res_find_obj_info(uint32_t base_addr, const char *partition, uint16_t img_id, img_res_obj_info_t *out)
{
    if (!partition || !out) {
        return -1;
    }

    img_res_head_wrapper_t w;
    memset(&w, 0, sizeof(w));
    int ret = img_res_flash_read_by_partition(partition, base_addr, &w, sizeof(w));
    if (ret != 0) {
        LOGE("img_res_find_obj_info: read head failed: ret=%d addr=0x%08X", ret, (unsigned)base_addr);
        return -2;
    }

    if (!img_res_magic_valid(w.head.magic)) {
        LOGE("img_res_find_obj_info: bad magic: 0x%08X", (unsigned)w.head.magic);
        return -3;
    }

    const uint16_t max_cnt = img_res_get_max_count();
    if (w.head.res_count == 0 || w.head.res_count > max_cnt) {
        LOGE("img_res_find_obj_info: bad res_count=%u (max=%u)", (unsigned)w.head.res_count, (unsigned)max_cnt);
        return -4;
    }

    if (w.head.total_size < IMG_RES_HEADER_FIXED_SIZE) {
        LOGE("img_res_find_obj_info: bad total_size=0x%08X", (unsigned)w.head.total_size);
        return -5;
    }

    /// obj_info region starts at base + sizeof(wrapper)
    const uint32_t obj_info_total = (uint32_t)w.head.res_count * (uint32_t)sizeof(img_res_obj_info_t);
    uint32_t addr = base_addr + (uint32_t)sizeof(img_res_head_wrapper_t);
    uint32_t remaining = obj_info_total;

    /// small stack buffer to stream-search
    img_res_obj_info_t infos[16];
    while (remaining) {
        const uint32_t want = (remaining > sizeof(infos)) ? (uint32_t)sizeof(infos) : remaining;
        ret = img_res_flash_read_by_partition(partition, addr, infos, want);
        if (ret != 0) {
            LOGE("img_res_find_obj_info: read obj_info failed: ret=%d addr=0x%08X size=%u", ret, (unsigned)addr, (unsigned)want);
            return -6;
        }

        const uint32_t n = want / (uint32_t)sizeof(img_res_obj_info_t);
        for (uint32_t i = 0; i < n; i++) {
            if (infos[i].img_id == img_id) {
                memcpy(out, &infos[i], sizeof(*out));
                return 0;
            }
        }

        addr += want;
        remaining -= want;
    }

    return -7; /// not found
}

static int res_get_img_storage_info(uint16_t img_id, img_res_storage_info_t *info)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(info);

    uint32_t base_addr = 0;
    if (!img_res_get_base_addr(&base_addr)) {
        LOGE("res_get_img_storage_info: no base addr");
        return -2;
    }
    const char *partition = img_res_get_partition_name();
    img_res_obj_info_t obj;
    memset(&obj, 0, sizeof(obj));

    MUTEX_LOCK_FS(thiz->priv.m_fs_rw_lock, "res_get_img_storage_info");
    int ret = img_res_find_obj_info(base_addr, partition, img_id, &obj);
    MUTEX_UNLOCK_FS(thiz->priv.m_fs_rw_lock, "res_get_img_storage_info");
    if (ret != 0) {
        LOGE("res_get_img_storage_info: img_id=%u not found/err ret=%d", (unsigned)img_id, ret);
        return ret;
    }

    info->img_id = obj.img_id;
    info->width = obj.width;
    info->height = obj.height;
    info->x_offset = obj.x_offset;
    info->y_offset = obj.y_offset;
    info->data_size = obj.data_size;

    info->storage_addr = base_addr + (uint32_t)IMG_RES_HEADER_FIXED_SIZE + obj.offset_addr;
    return 0;
}

static int res_read_img_data_from_external(uint32_t ext_flash_addr, uint32_t size, uint8_t *buf)
{
    EXCEPTION_CHECK_INT(thiz);
    if (!buf || size == 0) {
        return -1;
    }

    /// NOTE: ext_flash_addr 期望是外部flash的绝对地址（同 platform_flash_read 使用方式）
    MUTEX_LOCK_FS(thiz->priv.m_fs_rw_lock, "res_read_img_data_from_external");
    int ret = platform_flash_read(PARTITION_IMG_EXT_DATA_NAME, ext_flash_addr, buf, size);
    MUTEX_UNLOCK_FS(thiz->priv.m_fs_rw_lock, "res_read_img_data_from_external");
    if (ret != 0) {
        LOGE("res_read_img_data_from_external: read failed ret=%d addr=0x%08X size=%u", ret, (unsigned)ext_flash_addr, (unsigned)size);
        return -2;
    }
    return 0;
}

static void img_res_dump_header(const img_res_head_wrapper_t *w)
{
    if (!w) {
        LOG_RES("img_res_dump_header: null");
        return;
    }

    LOG_RES_DEBUG("IMG RES @0x%08X (partition: %s)", (uint32_t)IMG_INT_DATA_START_ADDR, PARTITION_IMG_INT_DATA_NAME);
    LOG_RES_DEBUG("partition range: [0x%08X, 0x%08X) size=0x%08X",
            (uint32_t)IMG_INT_DATA_START_ADDR,
            (uint32_t)IMG_INT_DATA_END_ADDR,
            (uint32_t)((uint32_t)IMG_INT_DATA_END_ADDR - (uint32_t)IMG_INT_DATA_START_ADDR));
    LOG_RES_DEBUG("head.magic   = 0x%08X", (unsigned)w->head.magic);
    LOG_RES_DEBUG("head.crc16   = 0x%04X", (unsigned)w->head.crc16);
    LOG_RES_DEBUG("head.flags   = 0x%04X", (unsigned)w->head.flags);
    LOG_RES_DEBUG("head.type    = %u", (unsigned)w->head.type);
    LOG_RES_DEBUG("head.lang_id = %u", (unsigned)w->head.lang_id);
    LOG_RES_DEBUG("head.version = 0x%04X", (unsigned)w->head.version);
    LOG_RES_DEBUG("head.count   = %u", (unsigned)w->head.res_count);
    LOG_RES_DEBUG("head.total   = 0x%08X (%u)", (unsigned)w->head.total_size, (unsigned)w->head.total_size);
    LOG_RES_DEBUG("header_fixed_size (build) = %u", (unsigned)IMG_RES_HEADER_FIXED_SIZE);
#if IS_SUPPORT_RES_DEBUG_LOG
    LOG_HEX("img_md5_head", w->md5_head, IMG_RES_MD5_SIZE);
    LOG_HEX("img_md5_body", w->md5_body, IMG_RES_MD5_SIZE);
#endif
}

static int img_res_validate_and_dump(uint32_t base_addr)
{
    int ret;
    img_res_head_wrapper_t w;
    memset(&w, 0, sizeof(w));

    ret = img_res_flash_read(base_addr, &w, sizeof(w));
    if (ret != 0) {
        LOGE("img_res_flash_read head failed: ret=%d addr=0x%08X size=%u", ret, (unsigned)base_addr, (unsigned)sizeof(w));
        return -1;
    }

    img_res_dump_header(&w);

    bool ok = true;
    uint8_t *buf = NULL;
    const uint32_t buf_size = 1024;
    img_res_obj_info_t *infos = NULL;
    const uint32_t partition_size = (uint32_t)((uint32_t)IMG_INT_DATA_END_ADDR - (uint32_t)IMG_INT_DATA_START_ADDR);

    /// 1) magic
    const bool magic_ok = img_res_magic_valid(w.head.magic);
    LOG_RES("check.magic: %s", magic_ok ? "OK" : "FAIL");
    ok = ok && magic_ok;

    /// 2) version compatibility (same major)
    const bool ver_ok = img_res_version_compatible(w.head.version);
    LOG_RES("check.version(major): %s", ver_ok ? "OK" : "FAIL");
    ok = ok && ver_ok;

    /// 3) head crc16 (skip magic+crc16 field itself, same as pack_resources.py: head_desc[6:])
    const uint8_t *head_bytes = (const uint8_t *)&w.head;
    const uint32_t head_len = (uint32_t)sizeof(img_res_head_desc_t);
    const uint16_t crc_calc = (uint16_t)CRC16((head_bytes + 6), (head_len - 6));
    const bool crc_ok = (crc_calc == w.head.crc16);
    LOG_RES("check.head_crc16: %s (calc=0x%04X exp=0x%04X)", crc_ok ? "OK" : "FAIL", (unsigned)crc_calc, (unsigned)w.head.crc16);
    ok = ok && crc_ok;

    /// 4) count/size range
    const uint16_t max_cnt = img_res_get_max_count();
    const bool cnt_ok = (w.head.res_count <= max_cnt);
    LOG_RES("check.res_count: %s (count=%u max=%u)", cnt_ok ? "OK" : "FAIL", (unsigned)w.head.res_count, (unsigned)max_cnt);
    ok = ok && cnt_ok;

    const bool size_min_ok = (w.head.total_size >= IMG_RES_HEADER_FIXED_SIZE);
    const bool size_max_ok = (w.head.total_size <= partition_size);
    LOG_RES("check.total_size: %s (total=0x%08X, header_fixed=%u, partition=0x%08X)",
            (size_min_ok && size_max_ok) ? "OK" : "FAIL",
            (unsigned)w.head.total_size,
            (unsigned)IMG_RES_HEADER_FIXED_SIZE,
            (unsigned)partition_size);
    ok = ok && size_min_ok && size_max_ok;

    /// 5) head md5
    uint8_t md5_head_calc[IMG_RES_MD5_SIZE] = {0};
    MD5Context head_md5_ctx = {0};
    MD5_Init(&head_md5_ctx);
    MD5_Update(&head_md5_ctx, (unsigned char *)&w.head, (int)sizeof(w.head));
    MD5_Final(&head_md5_ctx, md5_head_calc);
    const bool md5_head_ok = (memcmp(md5_head_calc, w.md5_head, IMG_RES_MD5_SIZE) == 0);
    LOG_RES("check.md5_head: %s", md5_head_ok ? "OK" : "FAIL");
    if (!md5_head_ok) {
        LOG_HEX("md5_head_calc", md5_head_calc, IMG_RES_MD5_SIZE);
    }
    ok = ok && md5_head_ok;

    /// 6) body md5 (obj_info + res_body), streaming from flash
    const uint32_t obj_info_len = (uint32_t)w.head.res_count * (uint32_t)sizeof(img_res_obj_info_t);
    const uint32_t body_size = (w.head.total_size >= IMG_RES_HEADER_FIXED_SIZE) ? (uint32_t)(w.head.total_size - IMG_RES_HEADER_FIXED_SIZE) : 0;
    const uint32_t header_used = (uint32_t)img_res_calc_header_size(w.head.res_count);
    const bool header_fit_ok = (header_used <= IMG_RES_HEADER_FIXED_SIZE);
    LOG_RES("check.header_region_fit: %s (used=%u fixed=%u)",
            header_fit_ok ? "OK" : "FAIL", (unsigned)header_used, (unsigned)IMG_RES_HEADER_FIXED_SIZE);
    ok = ok && header_fit_ok;

    uint8_t md5_body_calc[IMG_RES_MD5_SIZE] = {0};
    MD5Context body_md5_ctx = {0};
    MD5_Init(&body_md5_ctx);

    buf = (uint8_t *)u_malloz(buf_size);
    if (!buf) {
        LOGE("u_malloz(%u) failed", (unsigned)buf_size);
        ok = false;
        goto out;
    }

    /// obj_info region starts at base + sizeof(wrapper)
    uint32_t remaining = obj_info_len;
    uint32_t addr = base_addr + (uint32_t)sizeof(img_res_head_wrapper_t);
    while (ok && remaining) {
        const uint32_t chunk = (remaining > buf_size) ? buf_size : remaining;
        ret = img_res_flash_read(addr, buf, chunk);
        if (ret != 0) {
            LOGE("img_res_flash_read obj_info failed: ret=%d addr=0x%08X size=%u", ret, (unsigned)addr, (unsigned)chunk);
            ok = false;
            break;
        }
        MD5_Update(&body_md5_ctx, buf, (int)chunk);
        addr += chunk;
        remaining -= chunk;
    }

    /// res_body starts at base + header_fixed_size (build-time constant)
    remaining = body_size;
    addr = base_addr + (uint32_t)IMG_RES_HEADER_FIXED_SIZE;
    while (ok && remaining) {
        const uint32_t chunk = (remaining > buf_size) ? buf_size : remaining;
        ret = img_res_flash_read(addr, buf, chunk);
        if (ret != 0) {
            LOGE("img_res_flash_read body failed: ret=%d addr=0x%08X size=%u", ret, (unsigned)addr, (unsigned)chunk);
            ok = false;
            break;
        }
        MD5_Update(&body_md5_ctx, buf, (int)chunk);
        addr += chunk;
        remaining -= chunk;
    }

    MD5_Final(&body_md5_ctx, md5_body_calc);

    const bool md5_body_ok = (memcmp(md5_body_calc, w.md5_body, IMG_RES_MD5_SIZE) == 0);
    LOG_RES("check.md5_body(obj_info+res_body): %s", md5_body_ok ? "OK" : "FAIL");
    if (!md5_body_ok) {
        LOG_HEX("md5_body_calc", md5_body_calc, IMG_RES_MD5_SIZE);
    }
    ok = ok && md5_body_ok;

    /// 7) dump first few obj_info items + (optional) verify first item's data crc16
    const uint16_t dump_n = (w.head.res_count > 5) ? 5 : w.head.res_count;
    if (dump_n) {
        infos = (img_res_obj_info_t *)u_malloz((uint32_t)dump_n * (uint32_t)sizeof(img_res_obj_info_t));
        if (!infos) {
            LOGE("u_malloz(%u) failed", (unsigned)((uint32_t)dump_n * (uint32_t)sizeof(img_res_obj_info_t)));
            ok = false;
            goto out;
        }
        ret = img_res_flash_read(base_addr + (uint32_t)sizeof(img_res_head_wrapper_t),
                                 infos,
                                 (uint32_t)dump_n * (uint32_t)sizeof(img_res_obj_info_t));
        if (ret == 0) {
            for (uint16_t i = 0; i < dump_n; i++) {
                LOG_RES_DEBUG("obj[%u]: id=%u crc16=0x%04X wh=%ux%u off=0x%08X size=%u xy=(%u,%u)",
                        (unsigned)i,
                        (unsigned)infos[i].img_id,
                        (unsigned)infos[i].crc16,
                        (unsigned)infos[i].width,
                        (unsigned)infos[i].height,
                        (unsigned)infos[i].offset_addr,
                        (unsigned)infos[i].data_size,
                        (unsigned)infos[i].x_offset,
                        (unsigned)infos[i].y_offset);
                if (body_size && (infos[i].offset_addr + infos[i].data_size > body_size)) {
                    LOG_RES("  -> WARN: obj[%u] out of body range (body_size=%u)", (unsigned)i, (unsigned)body_size);
                    ok = false;
                }
            }

            /// Verify first resource crc16 (streaming) to quickly sanity-check data placement.
            uint16_t data_crc = 0xFFFF;
            remaining = infos[0].data_size;
            addr = base_addr + (uint32_t)IMG_RES_HEADER_FIXED_SIZE + infos[0].offset_addr;
            while (ok && remaining) {
                const uint32_t chunk = (remaining > buf_size) ? buf_size : remaining;
                ret = img_res_flash_read(addr, buf, chunk);
                if (ret != 0) {
                    LOGE("img_res_flash_read obj0 data failed: ret=%d addr=0x%08X size=%u", ret, (unsigned)addr, (unsigned)chunk);
                    ok = false;
                    break;
                }
                CRC16_UPDATE(data_crc, buf, chunk);
                addr += chunk;
                remaining -= chunk;
            }
            const bool obj0_crc_ok = (data_crc == infos[0].crc16);
            LOG_RES("check.obj0_crc16(data): %s (calc=0x%04X exp=0x%04X)", obj0_crc_ok ? "OK" : "FAIL",
                    (unsigned)data_crc, (unsigned)infos[0].crc16);
            ok = ok && obj0_crc_ok;
        } else {
            LOGE("img_res_flash_read obj_info sample failed: ret=%d", ret);
            ok = false;
        }
    }

out:
#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_INTERNAL_FLASH
    LOG_RES("internal flash img res validation result: %s", ok ? "PASS" : "FAIL");
#else
    LOG_RES("external flash img res validation result: %s", ok ? "PASS" : "FAIL");
#endif
    if (infos) {
        u_free(infos);
        infos = NULL;
    }
    if (buf) {
        u_free(buf);
        buf = NULL;
    }
    return ok ? 0 : -1;
}


static int res_ui_img_data_load_impl(uint16_t img_id, res_img_info_t *out)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(out);
    if (out->id == img_id && out->data && out->size) {
        return 0;
    }
    img_res_storage_info_t info;
    memset(&info, 0, sizeof(info));
    int ret = res_get_img_storage_info(img_id, &info);
    if (ret != 0) {
        LOGE("===> get img storage info failed: img_id=%u ret=%d", (unsigned)img_id, ret);
        return ret;
    }

#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_INTERNAL_FLASH
    out->id = info.img_id;
    #if defined(UINTPTR_MAX) && (UINTPTR_MAX > 0xFFFFFFFFu)
        out->data = (uint8_t *)(uint64_t)info.storage_addr;
    #else
        out->data = (uint8_t *)(uintptr_t)info.storage_addr;
    #endif
    out->size = info.data_size;
    return 0;

#elif V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_EXTERNAL_FLASH
    /// resize / allocate buffer if needed
    if (out->data && out->size != info.data_size) {
        (void)u_free_psram(out->data, RES_PSRAM_TYPE);
        out->data = NULL;
        out->size = 0;
        out->id = 0;
    }

    if (!out->data) {
        out->data = (uint8_t *)u_malloz_psram(info.data_size, RES_PSRAM_TYPE);
        if (!out->data) {
            LOGE("res_ui_img_data_load: u_malloz_psram(%u) failed", (unsigned)info.data_size);
            return -3;
        }
    }

    ret = res_read_img_data_from_external(info.storage_addr, info.data_size, out->data);
    if (ret != 0) {
        LOGE("res_ui_img_data_load: read_img_data_from_external failed: img_id=%u ret=%d", (unsigned)img_id, ret);
        return ret;
    }

    out->id = info.img_id;
    out->size = info.data_size;
    return 0;

#else
    out->id = 0;
    out->data = NULL;
    out->size = 0;
    LOGE("res_ui_img_data_load: unsupported storage type");
    return -4;
#endif
}

static void res_ui_img_data_unload_impl(res_img_info_t *img)
{
    EXCEPTION_CHECK_VOID(img);
#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_EXTERNAL_FLASH
    if (img->data) {
        (void)u_free_psram(img->data, RES_PSRAM_TYPE);
        img->data = NULL;
    }
#endif
    memset(img, 0, sizeof(*img));
}

static void hdr_async_init_cb(hdr_post_args_t *args)
{
    EXCEPTION_CHECK_VOID(thiz);
    (void)img_res_validate_and_dump((uint32_t)IMG_INT_DATA_START_ADDR);
    if (!thiz->priv.res_init_completed) {
        thiz->priv.res_init_completed = true;
    }
}
#endif
 
static int wait_init_completed(void *res)
{
#if IS_SUPPORT_LVGL
    resource_manager_context_t *ctx = res;    
    uint32_t timeout = 0;
    while (timeout < 200) {
        timeout += 10;
        if (ctx->priv.res_init_completed) {
            return 0;
        }
        msleep(10);
    }
    LOGE("wait res init timeout (timeout=%u)", timeout);
    return -1;
#else
    return 0;
#endif
}

static int on_post_res_init(void *ptr) 
{
#if IS_SUPPORT_LVGL
    resource_manager_context_t *ctx = ptr;
    ctx->priv.res_init_completed = false;
    POST_TASK_HANDLER_NO_DELAY(hdr_async_init_cb, ptr, "async_init:res");
#endif
    return 0;
}

static int on_perpare_res_init(void *self)
{
    return 0;
}

static int uninstall_resource(int which)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_res_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, resource_manager_context_t);

    ptr->uninstall_resource = uninstall_resource;
    ptr->wait_init_completed = wait_init_completed;
#if IS_SUPPORT_LVGL
    ptr->res_ui_img_data_load = res_ui_img_data_load_impl;
    ptr->res_ui_img_data_unload = res_ui_img_data_unload_impl;
#endif
    MUTEX_INIT(&ptr->priv.m_fs_rw_lock, fs_rw_lock);
    thiz = ptr;

    return ADD_MODULE("res", ID_MODULE_RES, on_perpare_res_init, on_post_res_init, 
        ptr, list_head, sizeof(resource_manager_context_t));
}

