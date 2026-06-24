#ifndef __UI_BG_H__
#define __UI_BG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

struct ui_manager_context;
typedef struct ui_manager_context ui_manager_context_t;

void ui_bg_init(ui_manager_context_t *thiz);
void ui_bg_update(ui_manager_context_t *thiz, lv_obj_t *parent);
void ui_bg_deinit(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __UI_BG_H__
