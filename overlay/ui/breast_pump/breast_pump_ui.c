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
static void ui_fill_running_time(lcd_tlvs_t *tlv, uint16_t time_seconds)
{
    tlv->type = LCD_DEVICE_RUNNING_TIME;
    tlv->data_size = 2;
    tlv->data[0] = time_seconds >> 8;
    tlv->data[1] = time_seconds & 0xFF;
}

static void ui_fill_pump_level(lcd_tlvs_t *tlv, uint8_t level_left, uint8_t level_right)
{
    tlv->type = LCD_PUMP_LEVEL;
    tlv->data_size = 2;
    tlv->data[0] = level_left;
    tlv->data[1] = level_right;
}

static void ui_fill_pump_speed(lcd_tlvs_t *tlv, uint8_t speed)
{
    tlv->type = LCD_PUMP_SPEED;
    tlv->data_size = 1;
    tlv->data[0] = speed + 1;
    // LOG_UI("ui_fill_pump_speed: %d", tlv->data[0]);
}

static void ui_fill_pump_mode(lcd_tlvs_t *tlv, uint8_t mode)
{
    tlv->type = LCD_PUMP_MODE;
    tlv->data_size = 1;
    tlv->data[0] = mode + 1;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int ui_display_pump_home_screen(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    uint16_t tlvs_num = 8;
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t) * tlvs_num);
    EXCEPTION_CHECK_UINT(p);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    const pump_result_info_t *result = pump->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    uint16_t hold_time = result->work_valid_time / 1000;
    pump_setting_t setting;
    pump->get_breast_pump_setting(&setting);

    ui_fill_img_id(&p[0], UI_IMG_ID_HOME_SCREEN);
    ui_fill_running_time(&p[1], hold_time);
    ui_fill_pump_level(&p[2], setting.level[0], setting.level[1]);
    ui_fill_pump_speed(&p[3], setting.speed);
    ui_fill_ble_status(&p[4]);
    ui_fill_charge_status(&p[5]);
    ui_fill_battery_status(&p[6]);
    ui_fill_pump_mode(&p[7], pump->get_breast_pump_mode());
    *tlvs = p;
    // LOG_UI("pump_home_screen:%p, tlvs_num: %d", p, tlvs_num);
    return tlvs_num;
}

int ui_display_pump_left_level(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    uint16_t tlvs_num = 3;
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t) * tlvs_num);
    EXCEPTION_CHECK_UINT(p);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    const pump_result_info_t *result = pump->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    pump_setting_t setting;
    pump->get_breast_pump_setting(&setting);
    ui_fill_img_id(&p[0], UI_IMG_ID_LEFT_LEVEL_ADJUST_DISPLAY);
    ui_fill_pump_level(&p[1], setting.level[0], setting.level[1]);
    ui_fill_pump_speed(&p[2], setting.speed);
    *tlvs = p;
    // LOG_UI("pump_left_level_screen:%p, tlvs_num: %d", p, tlvs_num);
    return tlvs_num;
}

int ui_display_pump_right_level(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    uint16_t tlvs_num = 3;
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t) * tlvs_num);
    EXCEPTION_CHECK_UINT(p);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    const pump_result_info_t *result = pump->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    pump_setting_t setting;
    pump->get_breast_pump_setting(&setting);
    ui_fill_img_id(&p[0], UI_IMG_ID_RIGHT_LEVEL_ADJUST_DISPLAY);
    ui_fill_pump_level(&p[1], setting.level[0], setting.level[1]);
    ui_fill_pump_speed(&p[2], setting.speed);
    *tlvs = p;
    // LOG_UI("pump_right_level_screen:%p, tlvs_num: %d", p, tlvs_num);
    return tlvs_num;
}

int ui_display_pump_complete_tip(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t));
    EXCEPTION_CHECK_UINT(p);
    ui_fill_img_id(p, UI_IMG_ID_PUMP_COMPLETE_TIP);
    *tlvs = p;
    return 1;
}

int ui_display_pump_mode(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    uint16_t tlvs_num = 2;
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t) * tlvs_num);
    EXCEPTION_CHECK_UINT(p);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    uint8_t mode = pump->get_breast_pump_mode();
    ui_fill_img_id(&p[0],  UI_ID_PUMP_MODE_TIP);
    ui_fill_pump_mode(&p[1], mode);
    *tlvs = p;
    // LOG_UI("pump_mode_screen:%p, tlvs_num: %d", p, tlvs_num);
    return tlvs_num;
}

int ui_display_chasing_milk_mode(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    uint16_t tlvs_num = 2;
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t) * tlvs_num);
    EXCEPTION_CHECK_UINT(p);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    uint8_t mode = pump->get_breast_pump_mode();
    ui_fill_img_id(&p[0], UI_IMG_ID_SETTINGS_SUBMENU);
    ui_fill_pump_mode(&p[1], mode);
    *tlvs = p;
    // LOG_UI("chasing_milk_mode_screen:%p, tlvs_num: %d", p, tlvs_num);
    return tlvs_num;
}

int ui_display_pump_speed(ui_animation_play_t *ani, lcd_tlvs_t **tlvs)
{
    uint16_t tlvs_num = 2;
    lcd_tlvs_t *p = u_malloz(sizeof(lcd_tlvs_t) * tlvs_num);
    EXCEPTION_CHECK_UINT(p);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    pump_setting_t setting;
    pump->get_breast_pump_setting(&setting);
    ui_fill_img_id(&p[0], UI_IMG_ID_PUMP_SPEED);
    ui_fill_pump_speed(&p[1], setting.speed);
    *tlvs = p;
    return tlvs_num;
}