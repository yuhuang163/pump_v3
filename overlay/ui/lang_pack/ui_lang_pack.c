/* Shared FDS language-pack loader for v3_ui_v7 and v3pro_ui_v7. */
#include "ui_lang_pack.h"

#if IS_SUPPORT_LANG_PACK

#include "hal_database.h"
#include "product_info.h"
#include "resource_manager_api.h"
#include "root_common_api.h"

#if IS_SUPPORT_LVGL
#include "lvgl.h"
#endif

#include <string.h>

#define LOG_LANG_PACK(fmt, ...) LOG_UI("[lang_pack] " fmt, ##__VA_ARGS__)

static uint8_t s_pack_active;
static uint16_t s_pack_lang_id;
static lang_pack_header_t s_hdr;
static lang_manifest_entry_t s_entries[LANG_PACK_ENTRY_MAX];
static uint16_t s_entry_count;

static const char **s_str_ptr;
static uint16_t s_str_count;
static ui_widget_style_t *s_widgets;
static uint8_t *s_widget_ok;
static uint16_t s_widget_count;
static int16_t *s_layouts;
static uint8_t *s_layout_ok;
static uint16_t s_layout_count;

#if IS_SUPPORT_LVGL
static lv_font_t *s_pack_fonts[UI_FONT_ROLE_COUNT];
static uint8_t *s_font_blob[UI_FONT_ROLE_COUNT];
#endif
static ui_img_data_t s_pack_imgs[UI_FONT_IMG_COUNT];
static uint8_t *s_img_blob[UI_FONT_IMG_COUNT];
static uint8_t *s_page_blobs[LANG_PACK_ENTRY_MAX];
static uint16_t s_page_blob_count;

static void *lang_pack_ram_alloc(uint32_t size)
{
#if PSRAM_ENABLE
    return u_malloz_psram(size, RES_PSRAM_TYPE);
#else
    return u_malloz(size);
#endif
}

static void lang_pack_ram_free(void *p)
{
    if (!p) {
        return;
    }
#if PSRAM_ENABLE
    (void)u_free_psram(p, RES_PSRAM_TYPE);
#else
    u_free(p);
#endif
}

static rt_device_node_t *lang_pack_db_dev(void)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);

    if (!pbus || !pbus->find_device) {
        return NULL;
    }
    return (rt_device_node_t *)pbus->find_device(ID_DEVICE_DB);
}

static int lang_pack_read_ext(uint32_t key, void *buffer, uint32_t size)
{
    rt_device_node_t *db_dev = lang_pack_db_dev();

    if (!db_dev || !db_dev->read || !buffer || size == 0) {
        return -1;
    }

    uint32_t args[] = {key, (uint32_t)(uintptr_t)buffer, size, DB_STORAGE_EXTERNAL};
    return DEVICE_READ_ARGS(db_dev, args, sizeof(args));
}

static int lang_pack_write_ext(uint32_t key, const void *buffer, uint32_t size)
{
    rt_device_node_t *db_dev = lang_pack_db_dev();

    if (!db_dev || !db_dev->write || !buffer || size == 0) {
        return -1;
    }

    uint32_t args[] = {key, (uint32_t)(uintptr_t)buffer, size, DB_STORAGE_EXTERNAL};
    return DEVICE_WRITE(db_dev, args, sizeof(args));
}

static int lang_pack_delete_ext(uint32_t key)
{
    rt_device_node_t *db_dev = lang_pack_db_dev();

    if (!db_dev || !db_dev->ioctl) {
        return -1;
    }
    return DEVICE_IOCTL_ARGS(db_dev, OP_CODE_DB_EXT_DELETE, &key);
}

static uint32_t lang_pack_ready_crc(const lang_pack_ready_t *ready)
{
    return CRC32(ready, sizeof(*ready) - sizeof(ready->flag_crc32));
}

static int lang_pack_ready_valid(const lang_pack_ready_t *ready)
{
    if (!ready || ready->magic != LANG_READY_MAGIC) {
        return 0;
    }
    if (lang_pack_ready_crc(ready) != ready->flag_crc32) {
        return 0;
    }
    return 1;
}

static int lang_pack_ready_matches_hdr(const lang_pack_ready_t *ready,
                                       const lang_pack_header_t *hdr)
{
    if (ready->hdr_magic != hdr->magic) {
        return 0;
    }
    if (ready->pack_version != hdr->pack_version) {
        return 0;
    }
    if (ready->payload_size != hdr->payload_size) {
        return 0;
    }
    if (ready->payload_crc32 != hdr->payload_crc32) {
        return 0;
    }
    if (ready->lang_id != hdr->lang_id) {
        return 0;
    }
    if (ready->schema_major != hdr->schema_major) {
        return 0;
    }
    return 1;
}

int ui_lang_pack_is_header_compatible(const lang_pack_header_t *hdr)
{
    uint32_t entries_size;

    if (!hdr) {
        return 0;
    }
    if (hdr->magic != LANG_PACK_MAGIC) {
        return 0;
    }
    if (hdr->schema_major != UI_LANG_SCHEMA_MAJOR) {
        return 0;
    }
    if (hdr->product_id != (uint16_t)ROOT_PRODUCT_ID) {
        return 0;
    }
    if (hdr->ui_version != (uint16_t)ROOT_UI_VERSION) {
        return 0;
    }
    if (hdr->entry_count == 0 || hdr->entry_count > LANG_PACK_ENTRY_MAX) {
        return 0;
    }
    entries_size = (uint32_t)hdr->entry_count * sizeof(lang_manifest_entry_t);
    if (hdr->manifest_size != sizeof(lang_pack_header_t) + entries_size) {
        return 0;
    }
    if (hdr->payload_size < entries_size) {
        return 0;
    }
    return 1;
}

static int lang_pack_font_from_fs(ui_font_role_e role)
{
    return (role <= UI_FONT_ROLE_TITLE_LG);
}

static void lang_pack_reset(void)
{
    uint32_t i;

    s_pack_active = 0;
    s_pack_lang_id = 0;
    s_entry_count = 0;
    s_str_count = 0;
    s_widget_count = 0;
    s_layout_count = 0;
    memset(&s_hdr, 0, sizeof(s_hdr));
    memset(s_entries, 0, sizeof(s_entries));

    for (i = 0; i < (uint32_t)UI_FONT_IMG_COUNT; i++) {
        s_pack_imgs[i].data = NULL;
        s_pack_imgs[i].size = 0;
        if (s_img_blob[i]) {
            lang_pack_ram_free(s_img_blob[i]);
            s_img_blob[i] = NULL;
        }
    }
#if IS_SUPPORT_LVGL
    for (i = 0; i < (uint32_t)UI_FONT_ROLE_COUNT; i++) {
        if (s_pack_fonts[i]) {
            lv_binfont_destroy(s_pack_fonts[i]);
            s_pack_fonts[i] = NULL;
        }
        if (s_font_blob[i]) {
            lang_pack_ram_free(s_font_blob[i]);
            s_font_blob[i] = NULL;
        }
    }
#endif
    for (i = 0; i < s_page_blob_count; i++) {
        if (s_page_blobs[i]) {
            lang_pack_ram_free(s_page_blobs[i]);
            s_page_blobs[i] = NULL;
        }
    }
    s_page_blob_count = 0;
    if (s_str_ptr) {
        lang_pack_ram_free(s_str_ptr);
        s_str_ptr = NULL;
    }
    if (s_widgets) {
        lang_pack_ram_free(s_widgets);
        s_widgets = NULL;
    }
    if (s_widget_ok) {
        lang_pack_ram_free(s_widget_ok);
        s_widget_ok = NULL;
    }
    if (s_layouts) {
        lang_pack_ram_free(s_layouts);
        s_layouts = NULL;
    }
    if (s_layout_ok) {
        lang_pack_ram_free(s_layout_ok);
        s_layout_ok = NULL;
    }
}

int ui_lang_pack_erase_ready(void)
{
    return lang_pack_delete_ext(LANG_READY_FDS_KEY);
}

int ui_lang_pack_delete_pack(void)
{
    lang_pack_header_t hdr;
    lang_manifest_entry_t entry;
    uint16_t i;

    if (lang_pack_read_ext(LANG_PACK_FDS_KEY, &hdr, sizeof(hdr)) == 0 &&
        ui_lang_pack_is_header_compatible(&hdr)) {
        for (i = 0; i < hdr.entry_count; i++) {
            uint32_t off = sizeof(hdr) + (uint32_t)i * sizeof(entry);

            if (ui_lang_pack_read_at(off, &entry, sizeof(entry)) == 0) {
                (void)lang_pack_delete_ext(entry.fds_key);
            }
        }
    }
    return lang_pack_delete_ext(LANG_PACK_FDS_KEY);
}

int ui_lang_pack_write_at(uint32_t offset, const void *data, uint32_t size)
{
    rt_device_node_t *db_dev = lang_pack_db_dev();
    db_ext_rw_args_t args;

    if (!db_dev || !db_dev->ioctl || !data || size == 0) {
        return -1;
    }
    args.key = LANG_PACK_FDS_KEY;
    args.offset = offset;
    args.buffer = (void *)(uintptr_t)data;
    args.size = size;
    return DEVICE_IOCTL_ARGS(db_dev, OP_CODE_DB_EXT_WRITE_AT, &args);
}

int ui_lang_pack_read_at(uint32_t offset, void *data, uint32_t size)
{
    rt_device_node_t *db_dev = lang_pack_db_dev();
    db_ext_rw_args_t args;

    if (!db_dev || !db_dev->ioctl || !data || size == 0) {
        return -1;
    }
    args.key = LANG_PACK_FDS_KEY;
    args.offset = offset;
    args.buffer = data;
    args.size = size;
    return DEVICE_IOCTL_ARGS(db_dev, OP_CODE_DB_EXT_READ_AT, &args);
}

int ui_lang_pack_write_ready(const lang_pack_header_t *hdr)
{
    lang_pack_ready_t ready = {0};

    if (!hdr) {
        return -1;
    }
    ready.magic = LANG_READY_MAGIC;
    ready.hdr_magic = hdr->magic;
    ready.pack_version = hdr->pack_version;
    ready.payload_size = hdr->payload_size;
    ready.payload_crc32 = hdr->payload_crc32;
    ready.lang_id = hdr->lang_id;
    ready.schema_major = hdr->schema_major;
    ready.flag_crc32 = lang_pack_ready_crc(&ready);
    return lang_pack_write_ext(LANG_READY_FDS_KEY, &ready, sizeof(ready));
}

int ui_lang_pack_install_container(void)
{
    lang_pack_header_t hdr;
    lang_manifest_entry_t entries[LANG_PACK_ENTRY_MAX];
    uint8_t *chunk;
    uint32_t data_off;
    uint16_t i;

    if (ui_lang_pack_read_at(0, &hdr, sizeof(hdr)) != 0 ||
        !ui_lang_pack_is_header_compatible(&hdr)) {
        return -1;
    }
    if (ui_lang_pack_read_at(sizeof(hdr), entries,
                             (uint32_t)hdr.entry_count * sizeof(entries[0])) != 0) {
        return -1;
    }

    data_off = hdr.manifest_size;
    for (i = 0; i < hdr.entry_count; i++) {
        uint8_t *obj;

        if (entries[i].size == 0 || entries[i].size > LANG_PACK_RAM_MAX ||
            data_off + entries[i].size > sizeof(hdr) + hdr.payload_size) {
            return -1;
        }
        obj = (uint8_t *)lang_pack_ram_alloc(entries[i].size);
        if (!obj) {
            return -1;
        }
        if (ui_lang_pack_read_at(data_off, obj, entries[i].size) != 0) {
            lang_pack_ram_free(obj);
            return -1;
        }
        (void)lang_pack_delete_ext(entries[i].fds_key);
        if (lang_pack_write_ext(entries[i].fds_key, obj, entries[i].size) != 0) {
            lang_pack_ram_free(obj);
            return -1;
        }
        lang_pack_ram_free(obj);
        data_off += entries[i].size;
        LOG_LANG_PACK("install key=0x%08lX size=%lu", (unsigned long)entries[i].fds_key,
                      (unsigned long)entries[i].size);
    }

    chunk = (uint8_t *)lang_pack_ram_alloc(hdr.manifest_size);
    if (!chunk) {
        return -1;
    }
    memcpy(chunk, &hdr, sizeof(hdr));
    memcpy(chunk + sizeof(hdr), entries, (uint32_t)hdr.entry_count * sizeof(entries[0]));
    if (lang_pack_write_ext(LANG_PACK_FDS_KEY, chunk, hdr.manifest_size) != 0) {
        lang_pack_ram_free(chunk);
        return -1;
    }
    lang_pack_ram_free(chunk);
    return 0;
}

static int lang_pack_parse_item(const uint8_t *p, uint32_t remain, uint32_t *used,
                                const ui_lang_item_t **item, const char **text)
{
    const ui_lang_item_t *it;
    uint32_t total;

    if (remain < sizeof(*it)) {
        return -1;
    }
    it = (const ui_lang_item_t *)p;
    total = sizeof(*it) + it->text_bytes;
    total = (total + 3u) & ~3u;
    if (remain < total) {
        return -1;
    }
    *item = it;
    *text = it->text_bytes ? (const char *)(p + sizeof(*it)) : NULL;
    *used = total;
    return 0;
}

static void lang_pack_fill_style(ui_widget_style_t *dst, const ui_lang_item_t *item)
{
    dst->kind = item->kind;
    dst->obj_align = item->obj_align;
    dst->x = item->x;
    dst->y = item->y;
    dst->opa = item->opa;
    dst->text_align = item->text_align;
    dst->line_space = item->line_space;
    dst->font_role = item->font_role;
    dst->max_w = item->max_w;
    dst->color_rgb = item->color_rgb;
    dst->flags = item->flags;
    dst->rsv = item->rsv;
}

static int lang_pack_apply_page(uint8_t *buf, uint32_t size, uint8_t expect_kind)
{
    lang_page_file_t hdr;
    uint32_t off;
    uint16_t i;

    if (size < sizeof(hdr)) {
        return -1;
    }
    memcpy(&hdr, buf, sizeof(hdr));
    if (hdr.magic != ((expect_kind == LANG_OBJ_PAGE_STR) ? LANG_PAGE_STR_MAGIC : LANG_PAGE_WGT_MAGIC)) {
        return -1;
    }
    if (sizeof(hdr) + hdr.payload_bytes > size) {
        return -1;
    }
    off = sizeof(hdr);
    for (i = 0; i < hdr.item_count; i++) {
        const ui_lang_item_t *item;
        const char *text;
        uint32_t used = 0;

        if (lang_pack_parse_item(buf + off, size - off, &used, &item, &text) != 0) {
            return -1;
        }
        if (expect_kind == LANG_OBJ_PAGE_STR) {
            if (item->id < s_str_count && text) {
                s_str_ptr[item->id] = text;
            }
        } else if (item->kind == UI_WIDGET_KIND_SCALAR) {
            if (item->id < s_layout_count) {
                s_layouts[item->id] = item->x;
                s_layout_ok[item->id] = 1;
            }
        } else if (item->id < s_widget_count) {
            lang_pack_fill_style(&s_widgets[item->id], item);
            s_widget_ok[item->id] = 1;
        }
        off += used;
    }
    return 0;
}

static int lang_pack_load_objects(void)
{
    uint16_t i;

    s_str_count = (uint16_t)UI_STR_ID_COUNT;
    s_widget_count = (uint16_t)UI_W_COUNT;
    s_layout_count = (uint16_t)UI_LAY_COUNT;

    s_str_ptr = (const char **)lang_pack_ram_alloc(sizeof(char *) * s_str_count);
    s_widgets = (ui_widget_style_t *)lang_pack_ram_alloc(sizeof(ui_widget_style_t) * s_widget_count);
    s_widget_ok = (uint8_t *)lang_pack_ram_alloc(s_widget_count);
    s_layouts = (int16_t *)lang_pack_ram_alloc(sizeof(int16_t) * s_layout_count);
    s_layout_ok = (uint8_t *)lang_pack_ram_alloc(s_layout_count);
    if (!s_str_ptr || !s_widgets || !s_widget_ok || !s_layouts || !s_layout_ok) {
        return -1;
    }
    memset(s_str_ptr, 0, sizeof(char *) * s_str_count);
    memset(s_widgets, 0, sizeof(ui_widget_style_t) * s_widget_count);
    memset(s_widget_ok, 0, s_widget_count);
    memset(s_layouts, 0, sizeof(int16_t) * s_layout_count);
    memset(s_layout_ok, 0, s_layout_count);

    for (i = 0; i < s_entry_count; i++) {
        const lang_manifest_entry_t *e = &s_entries[i];
        uint8_t *buf;

        if (e->kind != LANG_OBJ_PAGE_STR && e->kind != LANG_OBJ_PAGE_WGT && e->kind != LANG_OBJ_IMG) {
            continue;
        }
        if (e->size == 0 || e->size > LANG_PACK_RAM_MAX) {
            continue;
        }
        buf = (uint8_t *)lang_pack_ram_alloc(e->size);
        if (!buf) {
            return -1;
        }
        if (lang_pack_read_ext(e->fds_key, buf, e->size) != 0) {
            lang_pack_ram_free(buf);
            LOG_LANG_PACK("read key=0x%08lX fail", (unsigned long)e->fds_key);
            return -1;
        }
        if (e->kind == LANG_OBJ_IMG) {
            if (e->id < (uint8_t)UI_FONT_IMG_COUNT) {
                s_img_blob[e->id] = buf;
                s_pack_imgs[e->id].data = buf;
                s_pack_imgs[e->id].size = e->size;
            } else {
                lang_pack_ram_free(buf);
            }
            continue;
        }
        if (s_page_blob_count >= LANG_PACK_ENTRY_MAX) {
            lang_pack_ram_free(buf);
            return -1;
        }
        s_page_blobs[s_page_blob_count++] = buf;
        if (lang_pack_apply_page(buf, e->size, e->kind) != 0) {
            LOG_LANG_PACK("page key=0x%08lX parse fail", (unsigned long)e->fds_key);
            return -1;
        }
    }
    return 0;
}

static int lang_pack_load_fonts(void)
{
#if !IS_SUPPORT_LVGL
    return 0;
#else
    uint16_t i;

    for (i = 0; i < s_entry_count; i++) {
        const lang_manifest_entry_t *e = &s_entries[i];
        ui_font_role_e role;
        lv_font_t *font;

        if (e->kind != LANG_OBJ_FONT) {
            continue;
        }
        role = (ui_font_role_e)e->id;
        if (!lang_pack_font_from_fs(role) || e->size == 0) {
            continue;
        }
        if (s_pack_fonts[role]) {
            continue;
        }
        s_font_blob[role] = (uint8_t *)lang_pack_ram_alloc(e->size);
        if (!s_font_blob[role]) {
            LOG_LANG_PACK("font role %u oom", (unsigned)role);
            continue;
        }
        if (lang_pack_read_ext(e->fds_key, s_font_blob[role], e->size) != 0) {
            lang_pack_ram_free(s_font_blob[role]);
            s_font_blob[role] = NULL;
            LOG_LANG_PACK("font role %u read fail", (unsigned)role);
            continue;
        }
        font = lv_binfont_create_from_buffer(s_font_blob[role], e->size);
        if (!font) {
            lang_pack_ram_free(s_font_blob[role]);
            s_font_blob[role] = NULL;
            LOG_LANG_PACK("font role %u binfont fail", (unsigned)role);
            continue;
        }
        font->fallback = (lv_font_t *)g_ui_font_default[role];
        s_pack_fonts[role] = font;
        LOG_LANG_PACK("font role %u loaded %lu", (unsigned)role, (unsigned long)e->size);
    }
    return 0;
#endif
}

static void lang_pack_sync_language(uint16_t lang_id)
{
    storage_manager_context_t *storage_ctx =
        (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);

    if (storage_ctx && storage_ctx->put_store_ram_val) {
        storage_ctx->put_store_ram_val(STORE_ID_SYS_LANGUAGE, (int)lang_id, 0, NULL);
    }
}

static int lang_pack_lang_is_container(const lang_pack_header_t *hdr)
{
    uint8_t probe;

    if (!hdr || hdr->manifest_size < sizeof(*hdr)) {
        return 0;
    }
    /* PACK.bin keeps object blobs after the manifest. Slim LANG (LFS burn) does
     * not; payload_size still includes those blobs so size comparison is useless. */
    return ui_lang_pack_read_at(hdr->manifest_size, &probe, 1) == 0 ? 1 : 0;
}

static int lang_pack_load_manifest(void)
{
    uint32_t entries_size;

    if (lang_pack_read_ext(LANG_PACK_FDS_KEY, &s_hdr, sizeof(s_hdr)) != 0) {
        return -1;
    }
    if (!ui_lang_pack_is_header_compatible(&s_hdr)) {
        return -1;
    }
    if (lang_pack_lang_is_container(&s_hdr)) {
        if (ui_lang_pack_install_container() != 0) {
            LOG_LANG_PACK("container explode fail");
            return -1;
        }
        if (lang_pack_read_ext(LANG_PACK_FDS_KEY, &s_hdr, sizeof(s_hdr)) != 0 ||
            !ui_lang_pack_is_header_compatible(&s_hdr)) {
            return -1;
        }
    }
    entries_size = (uint32_t)s_hdr.entry_count * sizeof(lang_manifest_entry_t);
    if (ui_lang_pack_read_at(sizeof(s_hdr), s_entries, entries_size) != 0) {
        return -1;
    }
    s_entry_count = s_hdr.entry_count;
    return 0;
}

int ui_lang_pack_probe(void)
{
    lang_pack_ready_t ready;

    lang_pack_reset();

    if (lang_pack_read_ext(LANG_READY_FDS_KEY, &ready, sizeof(ready)) != 0) {
        LOG_LANG_PACK("no ready flag, use firmware default");
        return 0;
    }
    if (!lang_pack_ready_valid(&ready)) {
        LOG_LANG_PACK("ready invalid, use firmware default");
        return 0;
    }
    if (lang_pack_load_manifest() != 0) {
        lang_pack_reset();
        LOG_LANG_PACK("manifest load fail, use firmware default");
        return 0;
    }
    if (!lang_pack_ready_matches_hdr(&ready, &s_hdr)) {
        lang_pack_reset();
        LOG_LANG_PACK("header/ready mismatch, use firmware default");
        return 0;
    }
    if (lang_pack_load_objects() != 0) {
        lang_pack_reset();
        LOG_LANG_PACK("object load fail, use firmware default");
        return 0;
    }

    s_pack_lang_id = s_hdr.lang_id;
    s_pack_active = 1;
    lang_pack_sync_language(s_hdr.lang_id);
    LOG_LANG_PACK("active lang=%u ver=%lu objects=%u", (unsigned)s_hdr.lang_id,
                  (unsigned long)s_hdr.pack_version, (unsigned)s_entry_count);
    return 1;
}

int ui_lang_pack_is_active(void)
{
    return s_pack_active ? 1 : 0;
}

uint16_t ui_lang_pack_lang_id(void)
{
    return s_pack_lang_id;
}

const char *ui_lang_pack_str(ui_str_id_e id)
{
    if (!s_pack_active || (uint32_t)id >= (uint32_t)s_str_count) {
        return NULL;
    }
    return s_str_ptr[id];
}

const ui_widget_style_t *ui_lang_pack_widget(ui_widget_id_e id)
{
    if (!s_pack_active || (uint32_t)id >= (uint32_t)s_widget_count || !s_widget_ok[id]) {
        return NULL;
    }
    return &s_widgets[id];
}

int ui_lang_pack_layout(ui_layout_id_e id, int16_t *out)
{
    if (!out || !s_pack_active || (uint32_t)id >= (uint32_t)s_layout_count || !s_layout_ok[id]) {
        return -1;
    }
    *out = s_layouts[id];
    return 0;
}

const struct _lv_font_t *ui_lang_pack_font(ui_font_role_e id)
{
#if !IS_SUPPORT_LVGL
    (void)id;
    return NULL;
#else
    if (!s_pack_active || (uint32_t)id >= (uint32_t)UI_FONT_ROLE_COUNT) {
        return NULL;
    }
    return s_pack_fonts[id];
#endif
}

const ui_img_data_t *ui_lang_pack_font_img(ui_font_img_id_e id)
{
    if (!s_pack_active || (uint32_t)id >= (uint32_t)UI_FONT_IMG_COUNT) {
        return NULL;
    }
    if (!s_pack_imgs[id].data || s_pack_imgs[id].size == 0) {
        return NULL;
    }
    return &s_pack_imgs[id];
}

void ui_lang_pack_load_runtime_fonts(void)
{
    if (!s_pack_active) {
        return;
    }
    (void)lang_pack_load_fonts();
}

#endif /* IS_SUPPORT_LANG_PACK */
