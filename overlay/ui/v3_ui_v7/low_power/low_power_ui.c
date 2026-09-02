/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_battery.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_battery_low_power_prepare(ui_manager_context_t *thiz, uint8_t show_countdown,
    const char *text)
{
    (void)thiz;
    (void)show_countdown;
    (void)text;
}

void ui_display_battery_low_power_restart_countdown(void)
{
}

void ui_display_battery_low_power_remove(ui_manager_context_t *thiz)
{
    (void)thiz;
}

int ui_display_battery_low_power_process(ui_manager_context_t *thiz)
{
    (void)thiz;
    return 0;
}
