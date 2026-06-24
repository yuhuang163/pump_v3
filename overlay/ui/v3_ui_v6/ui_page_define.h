#ifndef __UI_PAGE_DEFINE_H__
#define __UI_PAGE_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keepin
#if IS_SUPPORT_LVGL
#include "home_ui.h" // IWYU pragma: keepin
#include "home_screen_saver_ui.h" // IWYU pragma: keepin
#include "home_night_light_adjust_ui.h" // IWYU pragma: keepin
#include "battery_ui.h" // IWYU pragma: keepin
#include "charge_ui.h" // IWYU pragma: keepin
#include "ble_ui.h" // IWYU pragma: keepin
#include "breast_pump_ui.h" // IWYU pragma: keepin
#include "airplane_mode_ui.h" // IWYU pragma: keepin
#include "guide_ui.h" // IWYU pragma: keepin
#include "logo_ui.h" // IWYU pragma: keepin
#include "ota_ui.h" // IWYU pragma: keepin
#include "product_tests_ui.h" // IWYU pragma: keepin
#include "system_ui.h" // IWYU pragma: keepin
#include "ui_popup_window.h" // IWYU pragma: keep
#endif

typedef struct {
    void (*remove)(ui_manager_context_t *thiz);
    int (*process)(ui_manager_context_t *thiz);
} ui_page_ops_t;

typedef struct {
    void (*init)(ui_manager_context_t *thiz);
    void (*deinit)(ui_manager_context_t *thiz);
} ui_module_lifecycle_ops_t;

#ifdef __cplusplus
}
#endif

#endif 

