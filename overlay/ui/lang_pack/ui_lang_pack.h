#ifndef __UI_LANG_PACK_H__
#define __UI_LANG_PACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "project_support_overlay.h"
#include "ui_lang_runtime.h"
#include "ui_img_define.h"

#define LANG_PACK_MAGIC                             (0x474E414Cu) /* "LANG" */
#define LANG_READY_MAGIC                            (0x31594452u) /* "RDY1" */
#define LANG_PAGE_STR_MAGIC                         (0x53524731u) /* "SRG1" */
#define LANG_PAGE_WGT_MAGIC                         (0x57475431u) /* "WGT1" */

#define UI_LANG_SCHEMA_MAJOR                        (2u)
#define UI_LANG_SCHEMA_MINOR                        (0u)

#define LANG_PACK_FDS_KEY                           (0x4C414E47u) /* LANG: manifest / container */
#define LANG_READY_FDS_KEY                          (0x52445931u)
#define LANG_FONT_FDS_KEY(role)                     (0x464E5400u | ((uint32_t)(role) & 0xFFu))
#define LANG_IMG_FDS_KEY(id)                        (0x46494D00u | ((uint32_t)(id) & 0xFFu))
#define LANG_PAGE_STR_FDS_KEY(page)                 (0x50530000u | ((uint32_t)(page) & 0xFFFFu))
#define LANG_PAGE_WGT_FDS_KEY(page)                 (0x50570000u | ((uint32_t)(page) & 0xFFFFu))

#define LANG_OBJ_FONT                               (1u)
#define LANG_OBJ_IMG                                (2u)
#define LANG_OBJ_PAGE_STR                           (3u)
#define LANG_OBJ_PAGE_WGT                           (4u)

#define LANG_PACK_ENTRY_MAX                         (64u)
#define LANG_PACK_RAM_MAX                           (500u * 1024u)
#define LANG_PACK_OTA_MAX_SIZE                      (0x160000u)

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t schema_major;
    uint16_t schema_minor;
    uint16_t product_id;
    uint16_t ui_version;
    uint16_t lang_id;
    uint16_t entry_count;
    uint32_t pack_version;
    uint32_t payload_size;
    uint32_t payload_crc32;
    uint32_t manifest_size;
    uint32_t reserved[8];
} lang_pack_header_t;

typedef struct __attribute__((packed)) {
    uint32_t fds_key;
    uint32_t size;
    uint32_t crc32;
    uint8_t kind;
    uint8_t id;
    uint16_t rsv;
} lang_manifest_entry_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t hdr_magic;
    uint32_t pack_version;
    uint32_t payload_size;
    uint32_t payload_crc32;
    uint16_t lang_id;
    uint16_t schema_major;
    uint32_t flag_crc32;
} lang_pack_ready_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t page_id;
    uint16_t item_count;
    uint32_t payload_bytes;
    uint32_t crc32;
} lang_page_file_t;

typedef struct __attribute__((packed)) {
    uint16_t id;
    uint8_t kind;
    uint8_t font_role;
    int16_t x;
    int16_t y;
    uint16_t max_w;
    uint8_t obj_align;
    uint8_t text_align;
    int8_t line_space;
    uint8_t opa;
    uint16_t color_rgb;
    uint8_t flags;
    uint8_t rsv;
    uint16_t text_bytes;
} ui_lang_item_t;

#if IS_SUPPORT_LANG_PACK
int ui_lang_pack_probe(void);
int ui_lang_pack_is_active(void);
uint16_t ui_lang_pack_lang_id(void);
int ui_lang_pack_erase_ready(void);
int ui_lang_pack_write_ready(const lang_pack_header_t *hdr);
int ui_lang_pack_delete_pack(void);
int ui_lang_pack_write_at(uint32_t offset, const void *data, uint32_t size);
int ui_lang_pack_read_at(uint32_t offset, void *data, uint32_t size);
int ui_lang_pack_is_header_compatible(const lang_pack_header_t *hdr);
int ui_lang_pack_install_container(void);

const char *ui_lang_pack_str(ui_str_id_e id);
const ui_widget_style_t *ui_lang_pack_widget(ui_widget_id_e id);
int ui_lang_pack_layout(ui_layout_id_e id, int16_t *out);
const struct _lv_font_t *ui_lang_pack_font(ui_font_role_e role);
const ui_img_data_t *ui_lang_pack_font_img(ui_font_img_id_e id);
void ui_lang_pack_load_runtime_fonts(void);
#else
static inline int ui_lang_pack_probe(void) { return 0; }
static inline int ui_lang_pack_is_active(void) { return 0; }
static inline uint16_t ui_lang_pack_lang_id(void) { return 0; }
static inline const char *ui_lang_pack_str(ui_str_id_e id) { (void)id; return NULL; }
static inline const ui_widget_style_t *ui_lang_pack_widget(ui_widget_id_e id) { (void)id; return NULL; }
static inline int ui_lang_pack_layout(ui_layout_id_e id, int16_t *out) { (void)id; (void)out; return -1; }
static inline const struct _lv_font_t *ui_lang_pack_font(ui_font_role_e role) { (void)role; return NULL; }
static inline const ui_img_data_t *ui_lang_pack_font_img(ui_font_img_id_e id) { (void)id; return NULL; }
static inline void ui_lang_pack_load_runtime_fonts(void) {}
#endif

#ifdef __cplusplus
}
#endif
#endif /* __UI_LANG_PACK_H__ */
