#ifndef __UI_PAGE_ID_H__
#define __UI_PAGE_ID_H__

/* 语言包按页拆文件。FDS：PAGE_STR=0x50530000|page，PAGE_WGT=0x50570000|page */
/* 本文件由 gen_ui_lang_tables.py 生成，不要手改。对照表见同目录 ui_lang_id_map.html */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_PAGE_COMMON = 0, /* 0x0000 */
    UI_PAGE_HOME, /* 0x0001 */
    UI_PAGE_PUMP, /* 0x0002 */
    UI_PAGE_GUIDE, /* 0x0003 */
    UI_PAGE_THEME_GUIDE, /* 0x0004 */
    UI_PAGE_RHYTHM, /* 0x0005 */
    UI_PAGE_SETTINGS, /* 0x0006 */
    UI_PAGE_POWER, /* 0x0007 */
    UI_PAGE_CHARGE, /* 0x0008 */
    UI_PAGE_OTA, /* 0x0009 */
    UI_PAGE_FACTORY, /* 0x000A */
    UI_PAGE_SYS_ERROR, /* 0x000B */
    UI_PAGE_BLE, /* 0x000C */
    UI_PAGE_BATTERY, /* 0x000D */
    UI_PAGE_SEAL, /* 0x000E */
    UI_PAGE_PRODUCT_TESTS, /* 0x000F */
    UI_PAGE_COUNT
} ui_page_id_e;

#ifdef __cplusplus
}
#endif
#endif /* __UI_PAGE_ID_H__ */
