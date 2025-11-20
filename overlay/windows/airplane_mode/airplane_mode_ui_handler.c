/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager_api.h"
#include "root_common.h"
#include "ui_img_define.h" // IWYU pragma: keep
#include "window_screen_airplane_mode.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define AIRPLANE_MODE_ANIMATION_FREQ                 (12)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const uint16_t s_airplane_mode_res_array[1] = {
    UI_IMG_ID_UNLOCK_TIP,
};

static const airplane_mode_led_status_t enter_airplane_mode_led_status[6] = {
    {PUMP_START_BL, LED_LEVEL_MAX / 2},
    {PUMP_MODE_BL, LED_LEVEL_MAX / 2},
    {PUMP_FREQ_SET_BL, 0},
    {PUMP_BREASTFEEDING_BL, 0},
    {PUMP_LEFT_CONTROL_BL, 0},
    {PUMP_RIGHT_CONTROL_BL, 0},
};

static const airplane_mode_led_status_t exit_airplane_mode_led_status[4] = {
    {PUMP_START_BL, LED_LEVEL_MAX},
    {PUMP_MODE_BL, LED_LEVEL_MAX},
    {PUMP_FREQ_SET_BL, LED_LEVEL_MAX},
    {PUMP_BREASTFEEDING_BL, LED_LEVEL_MAX},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void airplane_mode_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_airplane_mode_t *proc = (ui_proc_airplane_mode_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->wm && proc->wm->priv.misc);
    /// 如果充电状态发生变化，则刷新UI.
    if (proc->charging_status != proc->wm->priv.misc->is_charge_plug_in()) {
        proc->charging_status = proc->wm->priv.misc->is_charge_plug_in();
        goto UI_DISPLAY_REFRESH;
    }
    
    /// 如果上次动画时间小于更新间隔，则不刷新UI
    if (get_timestamp_ms(proc->last_progress_ani_time) < AIRPLANE_MODE_ANIMATION_UPDATE_INTERVAL) {
        return;
    }

    /// 如果电池电量百分比发生变化，则刷新UI.
    if (proc->battery_percent == proc->wm->priv.misc->get_battery_percent()) {
        return;
    }

UI_DISPLAY_REFRESH: 
    proc->battery_percent = proc->wm->priv.misc->get_battery_percent();
    proc->last_progress_ani_time = get_timestamp();
    if (ui->priv.play_info.reset_play_count_cb) {
        ui->priv.play_info.reset_play_count_cb(&ui->priv.play_info);
    }
}

static void airplane_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            airplane_mode_ui_policy(ui);
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
void start_airplane_mode_ui(ui_proc_airplane_mode_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "pumping_screen",
        .fps = AIRPLANE_MODE_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_airplane_mode_res_array,
        .res_count = 1,
        .progress_cb = airplane_mode_animation_progress_cb,
        .user_data = proc,
    };
    EXCEPTION_CHECK_VOID(proc->wm->priv.misc);
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
}

void stop_airplane_mode_ui(ui_proc_airplane_mode_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = NULL,
        .fps = AIRPLANE_MODE_ANIMATION_FREQ,
        .res_array = NULL,
        .res_count = 0,
        .progress_cb = NULL,
        .user_data = proc,
    };

    if (proc->wm && proc->wm->priv.misc) {
        proc->charging_status = proc->wm->priv.misc->is_charge_plug_in();
        proc->battery_percent = proc->wm->priv.misc->get_battery_percent();
    }
    ui->hook->ui_animation_play_init(ui, play_info);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}

void airplane_mode_led_policy(ui_proc_airplane_mode_t *proc, uint8_t status)
{
    uint8_t control_type = 0;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->wm && proc->wm->priv.pbus && proc->wm->priv.st);
    struct storage_manager_context *st = proc->wm->priv.st;
    rt_device_node_t *led_dev = proc->wm->priv.pbus->find_device(ID_DEVICE_LED);
    EXCEPTION_CHECK_VOID(led_dev);
    led_status_t led_status = {
        .mode = LED_MODE_LEVEL,
        .un.luminous.level = 0,
    };
    st->get_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &control_type);
    int args[2] = {PUMP_LED_ALL_ID, (int)&led_status};
    if (status) {
        for (uint8_t i = 0; i < LENGTH_OF_ARRAY(enter_airplane_mode_led_status); i++) {
            args[0] = enter_airplane_mode_led_status[i].id;
            led_status.un.luminous.level = enter_airplane_mode_led_status[i].level;
            led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
        }
    } else {
        for (uint8_t i = 0; i < LENGTH_OF_ARRAY(exit_airplane_mode_led_status); i++) {
            args[0] = exit_airplane_mode_led_status[i].id;
            led_status.un.luminous.level = exit_airplane_mode_led_status[i].level;
            led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
        }
        /// 设置左右控制背光灯
        args[0] = PUMP_LEFT_CONTROL_BL;
        led_status.un.luminous.level = control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? \
                    LED_LEVEL_MAX / 2 : LED_LEVEL_MAX;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
        led_status.un.luminous.level = control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT ? \
                    LED_LEVEL_MAX / 2 : LED_LEVEL_MAX;
        args[0] = PUMP_RIGHT_CONTROL_BL;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
    }
}

void airplane_mode_pump_policy(ui_proc_airplane_mode_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->wm && proc->wm->priv.pump);
    breast_pump_manager_context_t *pump = proc->wm->priv.pump;
    if (pump->is_on_working() > 0) {
        pump->set_breast_pump_status(BREAST_PUMP_FINISH);
    }
}
