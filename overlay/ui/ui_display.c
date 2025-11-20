/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"
#include "ui_page_define.h"
#include "ui_display.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static int ui_display_process_logo(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
static int ui_display_process_boot_qrcode(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
static int ui_display_process_unlock_tip(ui_animation_play_t *ani, lcd_tlvs_t **tlvs);
static ui_display_process_handle_t ui_display_process_list[] = {
    {UI_IMG_ID_LOGO, ui_display_process_logo},
    {UI_IMG_ID_BOOT_QRCODE, ui_display_process_boot_qrcode},
    {UI_IMG_ID_CHARGE, ui_display_charge_screen},
    {UI_IMG_ID_UNLOCK_TIP, ui_display_process_unlock_tip},
    {UI_ID_PUMP_MODE_TIP, ui_display_pump_mode},
    {UI_IMG_ID_PUMP_COMPLETE_TIP, ui_display_pump_complete_tip},
    {UI_IMG_ID_LEFT_LEVEL_ADJUST_DISPLAY, ui_display_pump_left_level},
    {UI_IMG_ID_RIGHT_LEVEL_ADJUST_DISPLAY, ui_display_pump_right_level},
    {UI_IMG_ID_SETTINGS_SUBMENU, ui_display_chasing_milk_mode},
    {UI_IMG_ID_HOME_SCREEN, ui_display_pump_home_screen},
    {UI_IMG_ID_PUMP_SPEED, ui_display_pump_speed},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int ui_display_process_logo(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t));
    EXCEPTION_CHECK_UINT(p);
    ui_fill_img_id(p, UI_IMG_ID_LOGO);
    LOG_UI("ui_display_process_logo: %d", UI_IMG_ID_LOGO);
    *tlvs = p;
    return 1;
}

static int ui_display_process_boot_qrcode(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t));
    EXCEPTION_CHECK_UINT(p);
    ui_fill_img_id(p, UI_IMG_ID_BOOT_QRCODE);
    *tlvs = p;
    return 1;
}


static int ui_display_process_unlock_tip(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t));
    EXCEPTION_CHECK_UINT(p);
    ui_fill_img_id(p, UI_IMG_ID_UNLOCK_TIP);
    *tlvs = p;
    return 1;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_fill_img_id(lcd_tlvs_t *tlv, uint16_t img_id)
{
    tlv->type = LCD_IMG_ID;
    tlv->data_size = 1;
    tlv->data[0] = img_id;
}

int lvgl_display_img_data_perpare(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{    
    EXCEPTION_CHECK_UINT(ani->res_array);
    int l = 0;
    int m = 0;
    uint16_t tmp = ani->res_array[ani->played_count];
    uint16_t img_id = 0;
    uint16_t list_len = LENGTH_OF_ARRAY(ui_display_process_list);
    ui_display_process_handle_t *list = ui_display_process_list;
    int h = list_len - 1;
    while (h >= l) {
        m = (h + l) / 2;
        img_id = list[m].img_id;
        if (tmp == img_id) {
            if (list[m].ui_display_process) {
                return list[m].ui_display_process(ani, tlvs);
            }
        } else if (tmp > img_id) {
            l = m + 1;
        } else if (tmp < img_id) {
            h = m - 1;
        }
    }
    LOGE("====>ui_display_process_list not found");
    return 0;
}
