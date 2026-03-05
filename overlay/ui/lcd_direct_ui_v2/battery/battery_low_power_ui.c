/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {    
    lv_obj_t * root;
    lv_obj_t * text;
    lv_obj_t * countdown;
    uint8_t countdown_value;
    uint8_t force_update;
    uint32_t start_time;
} battery_low_power_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static battery_low_power_display_info_t *low_power_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void battery_low_power_root_init(void)
{
   
}

static int battery_low_power_text_update(ui_manager_context_t *thiz)
{    
   
    return 0;
}

static int battery_low_power_countdown_update(ui_manager_context_t *thiz)
{
   
    
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_battery_low_power_prepare(ui_manager_context_t *thiz)
{

}

void ui_display_battery_low_power_remove(ui_manager_context_t *thiz)
{

}

int ui_display_battery_low_power_process(ui_manager_context_t *thiz)
{

    return 0;
}