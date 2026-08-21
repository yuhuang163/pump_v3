#ifndef __POWER_OFF_UI_H__
#define __POWER_OFF_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

/*
* GLOBAL DEFINITIONS
*****************************************************************************************
*/
typedef enum {
    POWER_OFF_TEXT_POWERING_OFF = 0,   /* Powering off... */
    POWER_OFF_TEXT_AUTO_SHUT_OFF,      /* Auto Powering Off... */
    POWER_OFF_TEXT_LOW_POWER,          /* Battery to low to\npower off... */
} power_off_text_type_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ui_display_power_off_prepare(ui_manager_context_t *thiz);
void ui_display_power_off_remove(ui_manager_context_t *thiz);
int ui_display_power_off_process(ui_manager_context_t *thiz, power_off_text_type_e text_type);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__
