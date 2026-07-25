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

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

struct ui_manager_context;
typedef struct ui_manager_context ui_manager_context_t;

void ui_img_resource_init(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info);
void ui_img_resource_init_with_theme(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info);
void ui_theme_img_resource_init(ui_manager_context_t *thiz, uint16_t id,
    res_theme_type_e theme_type, res_img_info_t *info);
void ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info);

void ui_text_place_in_circle(lv_obj_t *lab, uint16_t angle, uint16_t ring_r, uint8_t is_reverse);
void ui_text_place_in_circle_log(char *log, uint8_t is_left);
void ui_place_dot_on_arc(lv_obj_t *arc, lv_obj_t *dot, uint16_t angle_deg,
    uint16_t arc_w, uint16_t dot_d, float extra_inset, bool is_reverse);

#ifdef __cplusplus
}
#endif

#endif // __UI_IMG_RESOURCE_H__
