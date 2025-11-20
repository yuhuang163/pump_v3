/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "ui_img_define.h"
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/// 泵奶首页动画资源ID.
static const uint16_t s_pump_home_res_array[1] = {
    UI_IMG_ID_HOME_SCREEN,
};

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
int switch_pump_power_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump)
{
    struct storage_manager_context *st = pump->priv.st;
    EXCEPTION_CHECK_INT(st);
    peripheral_bus_context_t *pbus = pump->priv.pbus;
    EXCEPTION_CHECK_INT(pbus);
    struct rt_device_node *led_dev = pbus->find_device(ID_DEVICE_LED);
    EXCEPTION_CHECK_INT(led_dev);
    night_light_info_t night_light_info = {0};
    st->get_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_INFO, 0, 0, &night_light_info);
    if (night_light_info.adaptive) {
        LOG_UI("night light is adaptive, ignore.");
        return 0;
    }
    night_light_info.onoff = !night_light_info.onoff;
    if (night_light_info.level == 0) {
        night_light_info.level = 5;
    } 
    st->put_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_INFO, 0, 0, &night_light_info);
    led_status_t led_status = {
        .mode = LED_MODE_LEVEL,
        .un.luminous.level = night_light_info.onoff ? night_light_info.level : 0,
    };
    int args[] = {NIGHT_LIGHT_LED_ID_0, (int)&led_status};
    led_dev->ioctl(led_dev, OP_CODE_NIGHT_LIGHT_STATUS_SET, (void *)args);
    LOG_UI("switch pump power: %d, level: %d", night_light_info.onoff, night_light_info.level);
    
    return 0;
}

void stop_pump_ui_animation(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = NULL,
        .fps = PUMP_ANIMATION_FREQ,
        .res_array = NULL,
        .res_count = 0,
        .progress_cb = NULL,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
    pumping_ui_hdr_remove();
    pump_setting_ui_hdr_remove();
    pump_mode_ui_hdr_remove();
}