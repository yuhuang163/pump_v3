/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
    lv_obj_t * charge_icon; 
    res_img_info_t charge_icon_info;
    uint8_t force_update;
    uint8_t rsv[15];
} power_off_charge_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static power_off_charge_display_info_t *charge_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_power_off_charge_screen_prepare(ui_manager_context_t *thiz)
{

}

void ui_display_power_off_charge_screen_remove(ui_manager_context_t *thiz)
{

}

int ui_display_power_off_charge_screen_process(ui_manager_context_t *thiz)
{
    return 0;
}