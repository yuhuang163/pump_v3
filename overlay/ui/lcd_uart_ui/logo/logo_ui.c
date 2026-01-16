/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

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
void ui_display_logo_prepare(ui_manager_context_t *thiz)
{

}

void ui_display_logo_remove(ui_manager_context_t *thiz)
{

}

int ui_display_logo_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t)),
        .tlvs_num = 1,
    };
    EXCEPTION_CHECK_UINT(msg.tlvs);
    lcd_tlvs_t *tlv = &msg.tlvs[0];
    UI_FILL_IMG_ID(tlv, UI_IMG_ID_LOGO);
    LOG_UI("ui_display_logo_process: %d", UI_IMG_ID_LOGO);
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
    return 0;
}
