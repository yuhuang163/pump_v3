#ifndef __CHARGE_ADAPTER_CUSTOM_UI_INTERNAL_H__
#define __CHARGE_ADAPTER_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

#define CHARGE_ADAPTER_CUSTOM_TEXT_COUNT             (2)

typedef struct {
    const char *name;
    uint16_t text_y;
    const lv_font_t *font;
    uint32_t color;
    int16_t line_space;
} charge_adapter_custom_text_info_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background;
    lv_obj_t *charge_adapter_icon;
    lv_obj_t *texts[CHARGE_ADAPTER_CUSTOM_TEXT_COUNT];
    uint8_t force_update;
    uint8_t rsv[15];
} charge_adapter_custom_ui_info_t;

void charge_adapter_custom_ui_prepare(ui_manager_context_t *thiz);
void charge_adapter_custom_ui_remove(ui_manager_context_t *thiz);
int charge_adapter_custom_ui_process(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __CHARGE_ADAPTER_CUSTOM_UI_INTERNAL_H__
