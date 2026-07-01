#ifndef __PUMP_FINISH_CUSTOM_UI_INTERNAL_H__
#define __PUMP_FINISH_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "pump_finish_ui.h" // IWYU pragma: keep

#define PUMP_FINISH_CUSTOM_TIP_TEXT_COUNT          (2)

typedef struct {
    const char *name;
    uint16_t text_x;
    uint16_t text_y;
    const lv_font_t *font;
    uint32_t color;
} pump_finish_custom_text_info_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    lv_obj_t *icon;
    lv_obj_t *texts[PUMP_FINISH_CUSTOM_TIP_TEXT_COUNT];
    uint8_t force_update;
    uint8_t rsv[15];
} pump_finish_custom_ui_info_t;

void pump_finish_custom_ui_prepare(ui_manager_context_t *thiz);
void pump_finish_custom_ui_remove(ui_manager_context_t *thiz);
int pump_finish_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __PUMP_FINISH_CUSTOM_UI_INTERNAL_H__
