#ifndef __GUIDE_CUSTOM_UI_INTERNAL_H__
#define __GUIDE_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep

typedef guide_base_ui_info_t guide_custom_ui_info_t;

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
guide_custom_ui_info_t *guide_custom_info_get(void);

void guide_custom_ui_prepare(ui_manager_context_t *thiz);
void guide_custom_ui_remove(ui_manager_context_t *thiz);
int guide_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __GUIDE_CUSTOM_UI_INTERNAL_H__
