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
#include "home_level_adjust_ui.h" // IWYU pragma: keepin
#include "home_both_level_adjust_ui.h" // IWYU pragma: keepin
#include "home_speed_adjust_ui.h" // IWYU pragma: keepin
#include "pump_level_adjust_ui.h" // IWYU pragma: keepin
#include "pump_both_level_adjust_ui.h" // IWYU pragma: keepin
#include "pump_speed_adjust_ui.h" // IWYU pragma: keepin
#include "battery_ui.h" // IWYU pragma: keepin
#include "low_power_ui.h" // IWYU pragma: keepin
#include "charge_ui.h" // IWYU pragma: keepin
#include "charge_adapter_ui.h" // IWYU pragma: keepin
#include "power_off_charge_ui.h" // IWYU pragma: keepin
#include "ble_ui.h" // IWYU pragma: keepin
#include "pump_home_ui.h" // IWYU pragma: keepin
#include "pump_finish_ui.h" // IWYU pragma: keepin
#include "airplane_mode_ui.h" // IWYU pragma: keepin
#include "guide_ui.h" // IWYU pragma: keepin
#include "logo_ui.h" // IWYU pragma: keepin
#include "ota_ui.h" // IWYU pragma: keepin
#include "product_tests_ui.h" // IWYU pragma: keepin
#include "power_off_ui.h" // IWYU pragma: keepin
#include "power_off_hold_ui.h" // IWYU pragma: keepin
#include "factory_reset_ui.h" // IWYU pragma: keepin
#include "settings_success_ui.h" // IWYU pragma: keepin
#include "rhythm_confirm_ui.h" // IWYU pragma: keepin
#include "pump_break_tip_ui.h" // IWYU pragma: keepin
#include "pump_stop_hold_ui.h" // IWYU pragma: keepin
#include "pump_finish_timeout_ui.h" // IWYU pragma: keepin
#include "pump_break_end_ui.h" // IWYU pragma: keepin
#include "pump_will_finish_ui.h" // IWYU pragma: keepin
#include "system_abnormal_ui.h" // IWYU pragma: keepin
#include "ui_popup_window.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "ui_img_baj_decode.h" // IWYU pragma: keep
#include "ui_img_png_decode.h" // IWYU pragma: keep
#include "ui_img_jpg_decode.h" // IWYU pragma: keep
#include "ui_font_img_tip.h" // IWYU pragma: keep
#include "ui_wave.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_popup.h" // IWYU pragma: keep
#include "theme_guide_ui.h" // IWYU pragma: keepin
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

