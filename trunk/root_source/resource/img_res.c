/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "resource_manager_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "resource_manager.h" // IWYU pragma: keep
#include "external/md5/md5.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_RES_DEBUG_LOG        (0) 
#define IS_SUPPORT_IMG_CRC16_CHECK      (1)
#define IS_SUPPORT_IMG_MD5_CHECK        (0)

#if IS_SUPPORT_RES_DEBUG_LOG
#define LOG_RES_DEBUG(format, args...)   LOG_RES(format, ##args)
#else
#define LOG_RES_DEBUG(format, args...)
#endif  

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

static const char *img_res_get_partition_name(void)
{
#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_INTERNAL_FLASH
    return PARTITION_IMG_INT_DATA_NAME;
#else
    return PARTITION_IMG_EXT_DATA_NAME;
#endif
}

static const char *img_res_get_theme_name(res_theme_type_e theme_type)
{
    if (theme_type == RES_THEME_TYPE_CUSTOM) {
        return "custom";
    } else if (theme_type == RES_THEME_TYPE_BASE) {
        return "base";
    }
    return "unknown";
}

static bool img_res_get_base_addr(resource_manager_context_t *thiz, 
        res_theme_type_e theme_type, uint32_t *base_addr, uint32_t *size)
{
    EXCEPTION_CHECK_BOOL(base_addr);
#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_INTERNAL_FLASH
    if (theme_type == RES_THEME_TYPE_BASE) {
        *base_addr = (uint32_t)IMG_INT_DATA_START_ADDR;
        *size = (uint32_t)IMG_INT_DATA_SIZE;
    } else {
        return false;
    }
#else
    if (theme_type == RES_THEME_TYPE_BASE) {
        *base_addr = (uint32_t)BASE_IMG_RES_START_ADDR;
        *size = (uint32_t)BASE_IMG_RES_SIZE;
    } else if (theme_type == RES_THEME_TYPE_CUSTOM) {
        *base_addr = (uint32_t)THEME_IMG_RES_START_ADDR;
        *size = (uint32_t)THEME_IMG_RES_SIZE;
    } else {
        return false;
    }
#endif
    return true;
}

static int img_res_flash_read(uint32_t addr, void *buf, uint32_t size)
{
    const char *partition = img_res_get_partition_name();
    if (!partition || !buf || size == 0) {
        return -1;
    }
    return platform_flash_read(partition, addr, (uint8_t *)buf, size);
}

static int img_res_flash_read_by_partition(const char *partition, uint32_t addr, void *buf, uint32_t size)
{
    if (!partition || !buf || size == 0) {
        return -1;
    }
    return platform_flash_read(partition, addr, (uint8_t *)buf, size);
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
        LOGE("read head failed: ret=%d addr=0x%08X", ret, (unsigned)base_addr);
        return -2;
    }

    if (!img_res_magic_valid(w.head.magic)) {
        LOGE("bad magic: 0x%08X", (unsigned)w.head.magic);
        return -3;
    }

    const uint16_t max_cnt = img_res_get_max_count();
    if (w.head.res_count == 0 || w.head.res_count > max_cnt) {
        LOGE("bad res_count=%u (max=%u)", (unsigned)w.head.res_count, (unsigned)max_cnt);
        return -4;
    }

    if (w.head.total_size < IMG_RES_HEADER_FIXED_SIZE) {
        LOGE("bad total_size=0x%08X", (unsigned)w.head.total_size);
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
            LOGE("read obj_info failed: ret=%d addr=0x%08X size=%u", ret, (unsigned)addr, (unsigned)want);
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

static int res_get_img_storage_info(resource_manager_context_t *thiz, 
            res_theme_type_e theme_type, uint16_t img_id, img_res_storage_info_t *info)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(info);

    const char *theme_name = img_res_get_theme_name(theme_type);
    uint32_t base_addr = 0, res_size = 0;
    if (!img_res_get_base_addr(thiz, theme_type, &base_addr, &res_size)) {
        LOGE("get %s base addr failed", theme_name);
        return -2;
    }
    const char *partition = img_res_get_partition_name();
    img_res_obj_info_t obj;
    memset(&obj, 0, sizeof(obj));

    MUTEX_LOCK_FS(thiz->priv.m_fs_rw_lock, "find_obj_info");
    int ret = img_res_find_obj_info(base_addr, partition, img_id, &obj);
    MUTEX_UNLOCK_FS(thiz->priv.m_fs_rw_lock, "find_obj_info");
    if (ret != 0) {
        LOGE("img_id=%u not found/err ret=%d", (unsigned)img_id, ret);
        return ret;
    }

    info->img_id = obj.img_id;
    info->width = obj.width;
    info->height = obj.height;
    info->x_offset = obj.x_offset;
    info->y_offset = obj.y_offset;
    info->crc16 = obj.crc16;
    info->data_size = obj.data_size;

    info->storage_addr = base_addr + (uint32_t)IMG_RES_HEADER_FIXED_SIZE + obj.offset_addr;

    return 0;
}

static int res_read_img_data_from_external(resource_manager_context_t *thiz, uint32_t ext_flash_addr, uint32_t size, uint8_t *buf)
{
    EXCEPTION_CHECK_INT(thiz);
    if (!buf || size == 0) {
        return -1;
    }

    MUTEX_LOCK_FS(thiz->priv.m_fs_rw_lock, "read_img_data");
    int ret = platform_flash_read(PARTITION_IMG_EXT_DATA_NAME, ext_flash_addr, buf, size);
    MUTEX_UNLOCK_FS(thiz->priv.m_fs_rw_lock, "read_img_data");
    if (ret != 0) {
        LOGE("read failed ret=%d addr=0x%08X size=%u", ret, (unsigned)ext_flash_addr, (unsigned)size);
        return -2;
    }
    return 0;
}

static void img_res_dump_header(const img_res_head_wrapper_t *w, const char *theme_name, 
        uint32_t partition_start, uint32_t partition_end)
{
    if (!w) {
        LOG_RES("%s null header", theme_name);
        return;
    }

    LOG_RES_DEBUG("%s IMG RES @0x%08X (partition: %s)", theme_name, (unsigned)partition_start, img_res_get_partition_name());
    LOG_RES_DEBUG("partition range: [0x%08X, 0x%08X) size=0x%08X",
            (unsigned)partition_start,
            (unsigned)partition_end,
            (unsigned)(partition_end - partition_start));
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

static int img_res_validate_and_dump(resource_manager_context_t *thiz, res_theme_type_e theme_type)
{
    int ret;
    img_res_head_wrapper_t w;
    memset(&w, 0, sizeof(w));
    const char *theme_name = img_res_get_theme_name(theme_type);
    uint32_t base_addr = 0, partition_size = 0;
    if (!img_res_get_base_addr(thiz, theme_type, &base_addr, &partition_size)) {
        LOGE("get %s base addr failed", theme_name);
        return -1;
    }

    ret = img_res_flash_read(base_addr, &w, sizeof(w));
    if (ret != 0) {
        LOGE("read %s head failed: ret=%d addr=0x%08X size=%u", theme_name, ret, (unsigned)base_addr, (unsigned)sizeof(w));
        return -1;
    }

    img_res_dump_header(&w, theme_name, base_addr, base_addr + partition_size);

    bool ok = true;
    uint8_t *buf = NULL;
    const uint32_t buf_size = 1024;
    img_res_obj_info_t *infos = NULL;

    /// check magic
    const bool magic_ok = img_res_magic_valid(w.head.magic);
    LOG_RES("check.%s.magic: %s", theme_name, magic_ok ? "OK" : "FAIL");
    if (!magic_ok) {
        ok = false;
        goto out;
    }

    /// check version compatibility (same major)
    const bool ver_ok = img_res_version_compatible(w.head.version);
    LOG_RES("check.%s.version(major): %s", theme_name, ver_ok ? "OK" : "FAIL");
    ok = ok && ver_ok;

    /// check head crc16 (skip magic+crc16 field itself, same as pack_resources.py: head_desc[6:])
    const uint8_t *head_bytes = (const uint8_t *)&w.head;
    const uint32_t head_len = (uint32_t)sizeof(img_res_head_desc_t);
    const uint16_t crc_calc = (uint16_t)CRC16((head_bytes + 6), (head_len - 6));
    const bool crc_ok = (crc_calc == w.head.crc16);
    LOG_RES("check.%s.crc16(head): %s (calc=0x%04X exp=0x%04X)", theme_name, crc_ok ? "OK" : "FAIL", (unsigned)crc_calc, (unsigned)w.head.crc16);
    ok = ok && crc_ok;

    /// check res_count range
    const uint16_t max_cnt = img_res_get_max_count();
    const bool cnt_ok = (w.head.res_count <= max_cnt);
    LOG_RES("check.%s.res_count: %s (count=%u max=%u)", theme_name, cnt_ok ? "OK" : "FAIL", (unsigned)w.head.res_count, (unsigned)max_cnt);
    ok = ok && cnt_ok;

    const bool size_min_ok = (w.head.total_size >= IMG_RES_HEADER_FIXED_SIZE);
    const bool size_max_ok = (w.head.total_size <= partition_size);
    LOG_RES("check.%s.total_size: %s (total=0x%08X, header_fixed=%u, partition=0x%08X)",
            theme_name,
            (size_min_ok && size_max_ok) ? "OK" : "FAIL",
            (unsigned)w.head.total_size,
            (unsigned)IMG_RES_HEADER_FIXED_SIZE,
            (unsigned)partition_size);
    ok = ok && size_min_ok && size_max_ok;

    /// check head md5
#if IS_SUPPORT_IMG_MD5_CHECK
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
#endif

    /// check body md5 (obj_info + res_body), streaming from flash
    const uint32_t body_size = (w.head.total_size >= IMG_RES_HEADER_FIXED_SIZE) ? (uint32_t)(w.head.total_size - IMG_RES_HEADER_FIXED_SIZE) : 0;
    const uint32_t header_used = (uint32_t)img_res_calc_header_size(w.head.res_count);
    const bool header_fit_ok = (header_used <= IMG_RES_HEADER_FIXED_SIZE);
    LOG_RES("check.%s.header_region_fit: %s (used=%u fixed=%u)",
            theme_name,
            header_fit_ok ? "OK" : "FAIL", 
            (unsigned)header_used, (unsigned)IMG_RES_HEADER_FIXED_SIZE);
    ok = ok && header_fit_ok;

    /// Header fields are untrusted once any of the checks above failed.
    /// Skip body/obj sampling to avoid huge flash reads and misleading logs.
    if (!ok) {
        goto out;
    }

    uint32_t remaining = 0;
    uint32_t addr = 0;

    buf = (uint8_t *)u_malloz(buf_size);
    if (!buf) {
        LOGE("u_malloz(%u) failed", (unsigned)buf_size);
        ok = false;
        goto out;
    }

#if IS_SUPPORT_IMG_MD5_CHECK
    const uint32_t obj_info_len = (uint32_t)w.head.res_count * (uint32_t)sizeof(img_res_obj_info_t);
    uint8_t md5_body_calc[IMG_RES_MD5_SIZE] = {0};
    MD5Context body_md5_ctx = {0};
    MD5_Init(&body_md5_ctx);

    /// obj_info region starts at base + sizeof(wrapper)
    remaining = obj_info_len;
    addr = base_addr + (uint32_t)sizeof(img_res_head_wrapper_t);
    while (ok && remaining) {
        const uint32_t chunk = (remaining > buf_size) ? buf_size : remaining;
        ret = img_res_flash_read(addr, buf, chunk);
        if (ret != 0) {
            LOGE("read %s obj_info failed: ret=%d addr=0x%08X size=%u", theme_name, ret, (unsigned)addr, (unsigned)chunk);
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
            LOGE("read %s body failed: ret=%d addr=0x%08X size=%u", theme_name, ret, (unsigned)addr, (unsigned)chunk);
            ok = false;
            break;
        }
        MD5_Update(&body_md5_ctx, buf, (int)chunk);
        addr += chunk;
        remaining -= chunk;
    }

    MD5_Final(&body_md5_ctx, md5_body_calc);

    const bool md5_body_ok = (memcmp(md5_body_calc, w.md5_body, IMG_RES_MD5_SIZE) == 0);
    LOG_RES("check.%s.md5_body(obj_info+res_body): %s", theme_name, md5_body_ok ? "OK" : "FAIL");
    if (!md5_body_ok) {
        LOG_HEX("md5_body_calc", md5_body_calc, IMG_RES_MD5_SIZE);
    }
    ok = ok && md5_body_ok;
#endif

    /// dump first few obj_info items + (optional) verify first item's data crc16
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
                LOG_RES_DEBUG("%s.obj[%u]: id=%u crc16=0x%04X wh=%ux%u off=0x%08X size=%u xy=(%u,%u)",
                        theme_name,
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
                    LOG_RES("  -> WARN: %s.obj[%u] out of body range (body_size=%u)", theme_name, (unsigned)i, (unsigned)body_size);
                    ok = false;
                }
            }

            /// Verify first resource crc16 (streaming) to quickly sanity-check data placement.
            /// Skip when obj[0] is already out of range: initial crc 0xFFFF vs erased flash also 0xFFFF
            /// would otherwise report a false OK.
            if (ok) {
                uint16_t data_crc = 0xFFFF;
                remaining = infos[0].data_size;
                addr = base_addr + (uint32_t)IMG_RES_HEADER_FIXED_SIZE + infos[0].offset_addr;
                while (remaining) {
                    const uint32_t chunk = (remaining > buf_size) ? buf_size : remaining;
                    ret = img_res_flash_read(addr, buf, chunk);
                    if (ret != 0) {
                        LOGE("read %s data failed: ret=%d addr=0x%08X size=%u", theme_name, ret, (unsigned)addr, (unsigned)chunk);
                        ok = false;
                        break;
                    }
                    CRC16_UPDATE(data_crc, buf, chunk);
                    addr += chunk;
                    remaining -= chunk;
                }
                if (ok) {
                    const bool obj0_crc_ok = (data_crc == infos[0].crc16);
                    LOG_RES("check.%s.crc16(data): %s (calc=0x%04X exp=0x%04X)", theme_name, obj0_crc_ok ? "OK" : "FAIL",
                            (unsigned)data_crc, (unsigned)infos[0].crc16);
                    ok = ok && obj0_crc_ok;
                }
            }
        } else {
            LOGE("read %s obj_info sample failed: ret=%d", theme_name, ret);
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

    if (ok) {
        if (theme_type == RES_THEME_TYPE_BASE) {
            thiz->priv.img_base_res_version = w.head.version;
        } else if (theme_type == RES_THEME_TYPE_CUSTOM) {
            thiz->priv.img_custom_res_version = w.head.version;
        }
    }
    return ok ? 0 : -1;
}

static int res_data_load_with_theme_impl(resource_manager_context_t *thiz, res_theme_type_e theme_type, uint16_t img_id, res_img_info_t *out)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(out);
    if (out->id == img_id && out->data && out->size) {
        return 0;
    }
    img_res_storage_info_t info;
    memset(&info, 0, sizeof(info));
    const char *theme_name = img_res_get_theme_name(theme_type);
    int ret = res_get_img_storage_info(thiz, theme_type, img_id, &info);
    if (ret != 0) {
        LOGE("get %s img info failed: img_id=%u ret=%d", theme_name, (unsigned)img_id, ret);
        return ret;
    }

#if V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_INTERNAL_FLASH
    out->id = info.img_id;
    #if defined(UINTPTR_MAX) && (UINTPTR_MAX > 0xFFFFFFFFu)
        out->data = (uint8_t *)(uint64_t)info.storage_addr;
    #else
        out->data = (uint8_t *)(uintptr_t)info.storage_addr;
    #endif
    uint16_t data_crc16 = 0xFFFF;
    CRC16_UPDATE(data_crc16, out->data, info.data_size);
    if (data_crc16 != info.crc16) {
        LOGE("%s img crc mismatch: img_id=%u calc=0x%04X expect=0x%04X", theme_name,
                (unsigned)img_id, (unsigned)data_crc16, (unsigned)info.crc16);
        out->id = 0;
        out->crc16 = 0;
        out->data = NULL;
        out->size = 0;
        return -5;
    }
    out->crc16 = info.crc16;
    out->size = info.data_size;
    return 0;

#elif V3_IMG_RES_STORAGE_TYPE == IMG_RES_STORAGE_EXTERNAL_FLASH
    /// resize / allocate buffer if needed
    if (out->data && out->size != info.data_size) {
        (void)u_free_psram(out->data, RES_PSRAM_TYPE);
        out->data = NULL;
        out->size = 0;
        out->id = 0;
        out->crc16 = 0;
    }

    if (!out->data) {
        out->data = (uint8_t *)u_malloz_psram(info.data_size, RES_PSRAM_TYPE);
        if (!out->data) {
            LOGE("u_malloz_psram(%u) failed", (unsigned)info.data_size);
            return -3;
        }
    }

    ret = res_read_img_data_from_external(thiz, info.storage_addr, info.data_size, out->data);
    if (ret != 0) {
        LOGE("%s read_img_data_from_external failed: img_id=%u ret=%d", theme_name, (unsigned)img_id, ret);
        return ret;
    }

#if IS_SUPPORT_IMG_CRC16_CHECK
    uint16_t data_crc16 = 0xFFFF;
    CRC16_UPDATE(data_crc16, out->data, info.data_size);
    if (data_crc16 != info.crc16) {
        LOGE("%s img crc mismatch: img_id=%u calc=0x%04X expect=0x%04X", theme_name,
                (unsigned)img_id, (unsigned)data_crc16, (unsigned)info.crc16);
        return -5;
    }
#endif
    out->id = info.img_id;
    out->crc16 = data_crc16;
    out->size = info.data_size;
    return 0;
#else
    out->id = 0;
    out->crc16 = 0;
    out->data = NULL;
    out->size = 0;
    LOGE("%s unsupported storage type", theme_name);
    return -4;
#endif
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int img_res_init(resource_manager_context_t *thiz)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(thiz);
    MUTEX_LOCK_FS(thiz->priv.m_fs_rw_lock, "base_img_res_validate");
    ret = img_res_validate_and_dump(thiz, RES_THEME_TYPE_BASE);
    thiz->priv.base_res_init_completed = (ret == 0) ? true : false;
    MUTEX_UNLOCK_FS(thiz->priv.m_fs_rw_lock, "base_img_res_validate");
    if (ret != 0) {
        LOGE("validate base img res failed");
        return -1;
    }
    MUTEX_LOCK_FS(thiz->priv.m_fs_rw_lock, "theme_img_res_validate");
    ret = img_res_validate_and_dump(thiz, RES_THEME_TYPE_CUSTOM);
    thiz->priv.theme_res_init_completed = (ret == 0) ? true : false;
    MUTEX_UNLOCK_FS(thiz->priv.m_fs_rw_lock, "theme_img_res_validate");
    return 0;
}

int install_img_res(resource_manager_context_t *thiz, res_theme_type_e theme_type)
{
    int ret = 0;
    
    if (theme_type != RES_THEME_TYPE_CUSTOM) {
        LOGE("only support custom theme install");
        return 0;
    }
    MUTEX_LOCK_FS(thiz->priv.m_fs_rw_lock, "install_resource");
    ret = img_res_validate_and_dump(thiz, RES_THEME_TYPE_CUSTOM);
    thiz->priv.theme_res_init_completed = (ret == 0) ? true : false;
    MUTEX_UNLOCK_FS(thiz->priv.m_fs_rw_lock, "install_resource");
    if (ret != 0) {
        LOGE("install custom img res failed");
    }
    return ret;
}

int uninstall_img_res(resource_manager_context_t *thiz, res_theme_type_e theme_type)
{
    if (theme_type != RES_THEME_TYPE_CUSTOM) {
        LOGE("only support custom theme uninstall");
        return 0;
    }
    MUTEX_LOCK_FS(thiz->priv.m_fs_rw_lock, "uninstall_resource");
    thiz->priv.theme_res_init_completed = false;
    MUTEX_UNLOCK_FS(thiz->priv.m_fs_rw_lock, "uninstall_resource");

    return 0;
}

int res_data_load_with_theme(resource_manager_context_t *thiz, res_theme_type_e which, uint16_t img_id, res_img_info_t *out)
{
    if (which == RES_THEME_TYPE_CUSTOM && thiz->priv.theme_res_init_completed) {
        return res_data_load_with_theme_impl(thiz, RES_THEME_TYPE_CUSTOM, img_id, out);
    } 
    return res_data_load_with_theme_impl(thiz, RES_THEME_TYPE_BASE, img_id, out);
}


