#ifndef __GUIDE_BASE_UI_INTERNAL_H__
#define __GUIDE_BASE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_GUIDE_COMP_BG                                (0)
#define GUIDE_PAGE_IMG_NUM                                      (6)
#define GUIDE_QRCODE_BG_COLOR                                   0xFFFFFF
#define GUIDE_QRCODE_FG_COLOR                                   0x000000
#define GUIDE_QRCODE_BORDER_COLOR                               (0xFFEAE6)
#if 0
#define GUIDE_QRCODE_SIZE                                       (124 - 4)
#define GUIDE_QRCODE_ALIGN_Y                                    (68)
#else
#define GUIDE_QRCODE_SIZE                                       (124)
#define GUIDE_QRCODE_ALIGN_Y                                    (66)
#endif
#define GUIDE_TEXT_NUM                                          (4 + 2)
#define GUIDE_ARROW_TEXT_Y_OFFSET                               (316)
#define GUIDE_TEXT_COLOR                                        (0xB25962)
#define GUIDE_TITLE_TEXT_COLOR                                  (0xB25962)
#define GUIDE_DESC_TEXT_COLOR                                   (0xC5757C)
#define GUIDE_EXIT_TEXT_COLOR                                   (0xDCA7A5)
#define GUIDE_BIND_TEXT_COLOR                                   (0x6A5A5B)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * qrcode;
    lv_obj_t * text[GUIDE_TEXT_NUM];
    lv_obj_t * page_img;
    res_img_info_t img_info;
    uint8_t force_update;
    uint8_t page_index;
    uint8_t text_num;
    char qr_str[120];
} guide_base_ui_info_t;

typedef struct {
    const char *text;
    const lv_font_t *font;
    uint16_t text_x;
    uint16_t text_y;
    uint8_t line_space;
    uint32_t color;
    lv_opa_t text_opa;
} guide_text_info_t;

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
guide_base_ui_info_t *guide_base_info_get(void);

void guide_base_ui_prepare(ui_manager_context_t *thiz);
void guide_base_ui_remove(ui_manager_context_t *thiz);
int guide_base_ui_process(ui_manager_context_t *thiz);

void guide_base_page_ui_update(ui_manager_context_t *thiz, uint8_t page_index);
void guide_base_qrcode_ui_update(ui_manager_context_t *thiz, uint8_t page_index);
void guide_base_text_ui_update(ui_manager_context_t *thiz, uint8_t page_index);

#ifdef __cplusplus
}
#endif

#endif // __GUIDE_BASE_UI_INTERNAL_H__
