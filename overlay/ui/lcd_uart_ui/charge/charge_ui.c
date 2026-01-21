/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"

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

void ui_fill_charge_status(lcd_tlvs_t *tlv)
{
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    if (battery) {
        tlv->type = LCD_PUMP_CHARGE_STATUS;
        tlv->data_size = 1;
        tlv->data[0] = battery->get_charge_status();
    }
}

void ui_display_charge_screen_prepare(ui_manager_context_t *thiz)
{
}

void ui_display_charge_screen_remove(ui_manager_context_t *thiz)
{

}

int ui_display_charge_screen(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t) * 3),
        .tlvs_num = 3,
    };
    EXCEPTION_CHECK_UINT(msg.tlvs);
    lcd_tlvs_t *tlv = &msg.tlvs[0];
    UI_FILL_IMG_ID(tlv, UI_IMG_ID_CHARGE);
#if IS_SUPPORT_MODULE_BATTERY
    ui_fill_battery_status(&msg.tlvs[1]);
    ui_fill_charge_status(&msg.tlvs[2]);
#endif
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
    return 0;
}