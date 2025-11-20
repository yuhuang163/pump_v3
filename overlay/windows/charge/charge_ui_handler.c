/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/// 充电UI动画资源ID.
static const uint16_t s_charging_res_array[1] = {
    UI_IMG_ID_CHARGE,
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void charging_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_charge_t *proc = (ui_proc_charge_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    /// 如果上次动画时间小于更新间隔，则不刷新UI.
    if (get_timestamp_ms(proc->last_progress_ani_time) < CHARGE_ANIMATION_UPDATE_INTERVAL) {
        return;
    }
    /// 更新动画时间戳.
    proc->last_progress_ani_time = get_timestamp();
    /// 重置动画播放计数.
    if (ui->priv.play_info.reset_play_count_cb) {
        ui->priv.play_info.reset_play_count_cb(&ui->priv.play_info);
    }
}

static void charging_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            charging_ui_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            break;
        default:
            break;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int switch_charge_night_light_onoff_ui(ui_proc_charge_t *proc)
{
    struct storage_manager_context *st = proc->wm->priv.st;
    EXCEPTION_CHECK_INT(st);
    peripheral_bus_context_t *pbus = proc->wm->priv.pbus;
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
    LOG_UI("switch charge night light power: %d, level: %d", night_light_info.onoff, night_light_info.level);

    return 0;
}

void show_charging_ui(ui_proc_charge_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "charge_screen",
        .fps = CHARGE_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_charging_res_array,
        .res_count = 1,
        .progress_cb = charging_animation_progress_cb,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
}

void stop_charging_ui(ui_proc_charge_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = NULL,
        .fps = CHARGE_ANIMATION_FREQ,
        .res_array = NULL,
        .res_count = 0,
        .progress_cb = NULL,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}