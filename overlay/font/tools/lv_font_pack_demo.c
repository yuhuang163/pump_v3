/*
 * Demo for loading LVGL binary fonts from a packed file stored in external
 * littlefs. Keep this file as a reference or copy the needed functions into
 * the UI module that owns the labels.
 */

#include <stdint.h>
#include <string.h>

#include "hal_database.h"
#include "./lv_font_pack_manifest.h"
#include "lvgl.h" // IWYU pragma: keep
#include "root_common_api.h"

#ifndef LV_FONT_PACK_MAGIC
#define LV_FONT_PACK_MAGIC (0x5046564Cu)
#endif

#define FONT_PACK_CRC_OFFSET  (8u)

#ifndef LV_FONT_PACK_MAX_PACKAGE_SIZE
#define LV_FONT_PACK_MAX_PACKAGE_SIZE (0x100000u)
#endif

#ifndef LV_FONT_PACK_DB_ID
#define LV_FONT_PACK_DB_ID (0u)
#endif

#ifndef FONT_ID_QUICKSAND_96_BOLD
#define FONT_ID_QUICKSAND_96_BOLD (0x23C63A18u)
#endif

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t package_size;
    uint32_t package_crc32;
} font_pack_header_t;

typedef struct {
    uint32_t package_size;
    uint32_t checksum;
} lv_font_pack_info_t;

static uint32_t font_pack_crc32_update(uint32_t crc, const uint8_t *data, uint32_t size)
{
    while (size--) {
        crc ^= *data++;
        for (uint8_t bit = 0; bit < 8; bit++) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return crc;
}

static uint32_t font_pack_crc32(const uint8_t *data, uint32_t size)
{
    return font_pack_crc32_update(0xFFFFFFFFu, data, size) ^ 0xFFFFFFFFu;
}

static uint32_t font_pack_package_crc32(const uint8_t *package, uint32_t size)
{
    static const uint8_t zero_crc[sizeof(uint32_t)] = {0};
    uint32_t crc = 0xFFFFFFFFu;

    crc = font_pack_crc32_update(crc, package, FONT_PACK_CRC_OFFSET);
    crc = font_pack_crc32_update(crc, zero_crc, sizeof(zero_crc));
    crc = font_pack_crc32_update(crc,
                                 package + FONT_PACK_CRC_OFFSET + sizeof(uint32_t),
                                 size - FONT_PACK_CRC_OFFSET - sizeof(uint32_t));
    return crc ^ 0xFFFFFFFFu;
}

static int font_pack_validate_header(const font_pack_header_t *header)
{
    if (!header) {
        return -1;
    }
    if (header->magic != LV_FONT_PACK_MAGIC) {
        return -2;
    }
    if (header->package_size < sizeof(font_pack_header_t) ||
        header->package_size > LV_FONT_PACK_MAX_PACKAGE_SIZE) {
        return -4;
    }
    return 0;
}

static rt_device_node_t *font_pack_get_database_dev(void)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);

    if (!pbus || !pbus->find_device) {
        return NULL;
    }
    return (rt_device_node_t *)pbus->find_device(ID_DEVICE_DB);
}

static int font_pack_database_read_ext(uint32_t key, void *buffer, uint32_t size)
{
    rt_device_node_t *db_dev = font_pack_get_database_dev();

    if (!db_dev || !db_dev->read || !buffer || size == 0) {
        return -1;
    }

    uint32_t args[] = {key, (uint32_t)(uintptr_t)buffer, size, DB_STORAGE_EXTERNAL};
    return DEVICE_READ_ARGS(db_dev, args, sizeof(args));
}

static int font_pack_database_write_ext(uint32_t key, const void *buffer, uint32_t size)
{
    rt_device_node_t *db_dev = font_pack_get_database_dev();

    if (!db_dev || !db_dev->write || !buffer || size == 0) {
        return -1;
    }

    uint32_t args[] = {key, (uint32_t)(uintptr_t)buffer, size, DB_STORAGE_EXTERNAL};
    return DEVICE_WRITE(db_dev, args, sizeof(args));
}

static int font_pack_validate(const uint8_t *package, uint32_t size)
{
    const font_pack_header_t *header = (const font_pack_header_t *)package;

    if (!package || size < sizeof(font_pack_header_t)) {
        return -1;
    }
    if (font_pack_validate_header(header) != 0) {
        return -2;
    }
    if (header->package_size != size) {
        return -4;
    }
    if (font_pack_package_crc32(package, size) != header->package_crc32) {
        return -6;
    }

    return 0;
}

int lv_font_pack_demo_write_to_ext_lfs(uint32_t db_id, const uint8_t *package, uint32_t package_size)
{
    if (font_pack_validate(package, package_size) != 0) {
        return -1;
    }
    return font_pack_database_write_ext(db_id, package, package_size);
}

int lv_font_pack_demo_get_info(uint32_t db_id, lv_font_pack_info_t *info)
{
    font_pack_header_t header;

    if (!info) {
        return -1;
    }
    if (font_pack_database_read_ext(db_id, &header, sizeof(header)) != 0) {
        return -2;
    }
    if (font_pack_validate_header(&header) != 0) {
        return -3;
    }

    info->package_size = header.package_size;
    info->checksum = header.package_crc32;
    return 0;
}

lv_font_t *lv_font_pack_demo_load_from_key(uint32_t db_id)
{
    font_pack_header_t header;
    uint8_t *package = NULL;
    lv_font_t *font = NULL;

    if (font_pack_database_read_ext(db_id, &header, sizeof(header)) != 0) {
        return NULL;
    }
    if (font_pack_validate_header(&header) != 0) {
        return NULL;
    }

    package = lv_malloc(header.package_size);
    if (!package) {
        return NULL;
    }
    if (font_pack_database_read_ext(db_id, package, header.package_size) == 0 &&
        font_pack_validate(package, header.package_size) == 0) {
        uint32_t font_size = header.package_size - sizeof(font_pack_header_t);
        uint8_t *font_data = lv_malloc(font_size);
        if (font_data) {
            memcpy(font_data, package + sizeof(font_pack_header_t), font_size);
            font = lv_binfont_create_from_buffer(font_data, font_size);
            if (font) {
                font->fallback = lv_font_get_default();
            }
            lv_free(font_data);
        }
    }
    lv_free(package);
    return font;
}

lv_font_t *lv_font_pack_demo_load(const char *font_name)
{
    if (LV_FONT_PACK_DB_ID == 0u) {
        return NULL;
    }
    (void)font_name;
    return lv_font_pack_demo_load_from_key(LV_FONT_PACK_DB_ID);
}

void lv_font_pack_demo_destroy(lv_font_t *font)
{
    lv_binfont_destroy(font);
}

void lv_font_pack_demo_apply_to_label(lv_obj_t *label)
{
    lv_font_t *font = lv_font_pack_demo_load_from_key(FONT_ID_QUICKSAND_96_BOLD);

    if (font) {
        lv_obj_set_style_text_font(label, font, 0);
    } else {
        lv_obj_set_style_text_font(label, lv_font_get_default(), 0);
    }
    lv_label_set_text(label, "12:30");

    /*
     * Missing glyphs in the packed font will use lv_font_get_default().
     * Keep the returned font pointer while any object is using it. Call
     * lv_font_pack_demo_destroy(font) only after those objects no longer
     * reference the font.
     */
}
