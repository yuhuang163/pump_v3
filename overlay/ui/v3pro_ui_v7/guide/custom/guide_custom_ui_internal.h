#ifndef __GUIDE_CUSTOM_UI_INTERNAL_H__
#define __GUIDE_CUSTOM_UI_INTERNAL_H__

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
#define IS_SUPPORT_CUSTOM_GUIDE_COMP_BG            (0)
#define GUIDE_PAGE_INDEX_INVALID                   (0xFF)

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
void guide_custom_ui_prepare(ui_manager_context_t *thiz);
void guide_custom_ui_remove(ui_manager_context_t *thiz);
int guide_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __GUIDE_CUSTOM_UI_INTERNAL_H__
