#ifndef __UI_IMG_RESOURCE_H__
#define __UI_IMG_RESOURCE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep

struct ui_manager_context;
typedef struct ui_manager_context ui_manager_context_t;

void ui_theme_img_resource_init(ui_manager_context_t *thiz, uint16_t id,
    res_theme_type_e theme_type, res_img_info_t *info);
void ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info);

#ifdef __cplusplus
}
#endif

#endif // __UI_IMG_RESOURCE_H__
