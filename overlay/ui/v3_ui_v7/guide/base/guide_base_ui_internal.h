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
#define IS_SUPPORT_BASE_GUIDE_COMP_BG                   (0)
#define GUIDE_PAGE_INDEX_INVALID                        (0xFF)

#define IS_SUPPORT_BASE_GUIDE_LABEL_TEXT_CMP            (0) /// 支持label文本对比
#if IS_SUPPORT_BASE_GUIDE_LABEL_TEXT_CMP
#define GUIDE_BASE_LABEL_CARD_TEXT_COLOR                (0x00FF00)
#else
#define GUIDE_BASE_LABEL_CARD_TEXT_COLOR                (0xA0183B)
#endif

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
void guide_base_ui_prepare(ui_manager_context_t *thiz);
void guide_base_ui_remove(ui_manager_context_t *thiz);
int guide_base_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __GUIDE_BASE_UI_INTERNAL_H__
