/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "motor_manager_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"
#include "battery_ui.h"  // IWYU pragma: keep
#include "ble_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h"
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
void ui_display_pump_home_screen_prepare(ui_manager_context_t *thiz)
{
    // LOG_UI("ui_display_pump_home_screen_prepare");
}

void ui_display_pump_home_screen_remove(ui_manager_context_t *thiz)
{
    // LOG_UI("ui_display_pump_home_screen_remove");
}

int ui_display_pump_home_screen(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t) * 8),
        .tlvs_num = 8,
    };
    EXCEPTION_CHECK_UINT(msg.tlvs);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump);
    motor_manager_context_t *motor = pump->priv.motor;
    EXCEPTION_CHECK_UINT(motor);
    int time_index = motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    const pump_result_info_t *result = pump->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    uint16_t hold_time = result->work_valid_time / 1000;
    pump_setting_t setting;
    pump->get_breast_pump_setting(&setting, time_index);
    lcd_tlvs_t *tlv = &msg.tlvs[0];
    UI_FILL_IMG_ID(tlv, UI_IMG_ID_HOME_SCREEN);
    ui_fill_running_time(&msg.tlvs[1], hold_time);
    ui_fill_pump_level(&msg.tlvs[2], setting.level[0], setting.level[1]);
    ui_fill_pump_speed(&msg.tlvs[3], setting.speed);
    ui_fill_ble_status(&msg.tlvs[4]);
#if IS_SUPPORT_MODULE_BATTERY
    ui_fill_charge_status(&msg.tlvs[5]);
    ui_fill_battery_status(&msg.tlvs[6]);
#endif
    ui_fill_pump_mode(&msg.tlvs[7], pump->get_breast_pump_mode());
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
    return 0;
}

void ui_display_pump_level_prepare(ui_manager_context_t *thiz)
{
    // LOG_UI("ui_display_pump_left_level_prepare");
}

void ui_display_pump_level_remove(ui_manager_context_t *thiz)
{
    // LOG_UI("ui_display_pump_left_level_remove");
}

int ui_display_pump_level(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t) * 3),
        .tlvs_num = 3,
    };
    EXCEPTION_CHECK_UINT(msg.tlvs);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump); 
    motor_manager_context_t *motor = pump->priv.motor;
    EXCEPTION_CHECK_UINT(motor);
    int time_index = motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    const pump_result_info_t *result = pump->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    pump_setting_t setting;
    pump->get_breast_pump_setting(&setting, time_index);
    lcd_tlvs_t *tlv = &msg.tlvs[0];
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    uint8_t control_type = BREAST_PUMP_CONTROL_TYPE_BILATERAL;
    if (proc->st) {
        proc->st->get_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &control_type);
    }
    
    if (control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        UI_FILL_IMG_ID(tlv, UI_IMG_ID_LEFT_LEVEL_ADJUST_DISPLAY);
    } else {
        UI_FILL_IMG_ID(tlv, UI_IMG_ID_RIGHT_LEVEL_ADJUST_DISPLAY);
    }
    ui_fill_pump_level(&msg.tlvs[1], setting.level[0], setting.level[1]);
    ui_fill_pump_speed(&msg.tlvs[2], setting.speed);
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
    return 0;
}

void ui_display_pump_complete_tip_prepare(ui_manager_context_t *thiz)
{

}

void ui_display_pump_complete_tip_remove(ui_manager_context_t *thiz)
{

}

int ui_display_pump_complete_tip(ui_manager_context_t *thiz)
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
    UI_FILL_IMG_ID(tlv, UI_IMG_ID_PUMP_COMPLETE_TIP);
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
    return 0;
}

void ui_display_pump_mode_prepare(ui_manager_context_t *thiz)
{

}

void ui_display_pump_mode_remove(ui_manager_context_t *thiz)
{
}

int ui_display_pump_mode(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t) * 2),
        .tlvs_num = 2,
    };
    EXCEPTION_CHECK_UINT(msg.tlvs);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    uint8_t mode = pump->get_breast_pump_mode();
    lcd_tlvs_t *tlv = &msg.tlvs[0];
    UI_FILL_IMG_ID(tlv, UI_ID_PUMP_MODE_TIP);
    ui_fill_pump_mode(&msg.tlvs[1], mode);
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
    return 0;
}

void ui_display_breastfeeding_mode_prepare(ui_manager_context_t *thiz)
{
}

void ui_display_breastfeeding_mode_remove(ui_manager_context_t *thiz)
{
}

int ui_display_breastfeeding_mode(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t) * 2),
        .tlvs_num = 2,
    };
    EXCEPTION_CHECK_UINT(msg.tlvs);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    uint8_t mode = pump->get_breast_pump_mode();
    lcd_tlvs_t *tlv = &msg.tlvs[0];
    UI_FILL_IMG_ID(tlv, UI_IMG_ID_SETTINGS_SUBMENU);
    ui_fill_pump_mode(&msg.tlvs[1], mode);
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
    return 0;
}

void ui_display_pump_speed_prepare(ui_manager_context_t *thiz)
{
}

void ui_display_pump_speed_remove(ui_manager_context_t *thiz)
{
}

int ui_display_pump_speed(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_UINT(thiz);
    ui_animation_play_t *ani = &thiz->priv.play_info;
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t) * 2),
        .tlvs_num = 2,
    };
    EXCEPTION_CHECK_UINT(msg.tlvs);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_UINT(pump);
    motor_manager_context_t *motor = pump->priv.motor;
    EXCEPTION_CHECK_UINT(motor);
    int time_index = motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    pump_setting_t setting;
    pump->get_breast_pump_setting(&setting, time_index);
    lcd_tlvs_t *tlv = &msg.tlvs[0];
    UI_FILL_IMG_ID(tlv, UI_IMG_ID_PUMP_SPEED);
    ui_fill_pump_speed(&msg.tlvs[1], setting.speed);
    thiz->priv.panel_dev->ioctl(thiz->priv.panel_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
    return 0;
}