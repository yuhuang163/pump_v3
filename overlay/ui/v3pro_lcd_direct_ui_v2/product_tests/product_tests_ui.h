#ifndef __PRODUCT_TESTS_UI_H__
#define __PRODUCT_TESTS_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "window_screen_product_test.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ui_display_product_tests_home_prepare(ui_manager_context_t *thiz);
void ui_display_product_tests_home_remove(ui_manager_context_t *thiz);
int ui_display_product_tests_home_process(ui_manager_context_t *thiz);

void ui_display_lcd_color_test_prepare(ui_manager_context_t *thiz);
void ui_display_lcd_color_test_remove(ui_manager_context_t *thiz);
int ui_display_lcd_color_test_process(ui_manager_context_t *thiz);

void product_tip_popup_ui_update(ui_manager_context_t *thiz, ui_proc_product_test_t *proc,
    lv_obj_t *parent, uint8_t *ui_index);
void product_tip_popup_ui_remove(ui_manager_context_t *thiz);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

