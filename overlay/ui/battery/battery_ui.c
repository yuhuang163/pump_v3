/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"
#include "ui_page_define.h"
/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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
void ui_fill_battery_status(lcd_tlvs_t *tlv)
{
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    if (misc) {
        tlv->type = LCD_BATTERY_STATUS;
        tlv->data_size = 1;
        tlv->data[0] = misc->get_battery_percent();
    }
}

void ui_fill_charge_status(lcd_tlvs_t *tlv)
{
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    if (misc) {
        tlv->type = LCD_PUMP_CHARGE_STATUS;
        tlv->data_size = 1;
        tlv->data[0] = misc->get_charge_status();
    }
}

int ui_display_charge_screen(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    uint16_t tlvs_num = 3;
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t) * tlvs_num);
    EXCEPTION_CHECK_UINT(p);
    ui_fill_img_id(&p[0], UI_IMG_ID_CHARGE);
    ui_fill_battery_status(&p[1]);
    ui_fill_charge_status(&p[2]);
    *tlvs = p;

    return tlvs_num;
}