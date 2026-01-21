/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_airplane_mode.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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
static void airplane_mode_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void airplane_mode_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_airplane_mode_process(ui);
}

static void airplane_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            airplane_mode_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            airplane_mode_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void airplane_mode_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_airplane_mode_prepare(ui);
}

static void airplane_mode_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_airplane_mode_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_airplane_mode_ui(ui_proc_airplane_mode_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == AIRPLANE_MODE_UI_TYPE_HOME) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "airplane_mode_screen",
        .fps = AIRPLANE_MODE_ANIMATION_FREQ,
        .progress_cb = airplane_mode_animation_progress_cb,
        .display_prepare_cb = airplane_mode_ui_display_prepare,
        .display_remove_cb = airplane_mode_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = AIRPLANE_MODE_UI_TYPE_HOME;
}

void stop_airplane_mode_ui(ui_proc_airplane_mode_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}

void airplane_mode_led_policy(ui_proc_airplane_mode_t *proc, uint8_t status)
{
    uint8_t control_type = 0;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pbus);
    EXCEPTION_CHECK_VOID(proc->st);
    rt_device_node_t *led_dev = proc->pbus->find_device(ID_DEVICE_LED);
    EXCEPTION_CHECK_VOID(led_dev);
    led_status_msg_t msg = {
        .led_index = PUMP_LED_ALL_ID,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = 0,
        },
    };
    proc->st->get_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &control_type);
    if (status) {
        for (uint8_t i = 0; i < LENGTH_OF_ARRAY(enter_airplane_mode_led_status); i++) {
            msg.led_index = enter_airplane_mode_led_status[i].id;
            msg.led_status.un.luminous.level = enter_airplane_mode_led_status[i].level;
            led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
        }
    } else {
        for (uint8_t i = 0; i < LENGTH_OF_ARRAY(exit_airplane_mode_led_status); i++) {
            msg.led_index = exit_airplane_mode_led_status[i].id;
            msg.led_status.un.luminous.level = exit_airplane_mode_led_status[i].level;
            led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
        }
        /// 设置左右控制背光灯
        msg.led_index = PUMP_LEFT_CONTROL_BL;
        msg.led_status.un.luminous.level = control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? \
                    LED_LEVEL_MAX / 2 : LED_LEVEL_MAX;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
        msg.led_status.un.luminous.level = control_type == BREAST_PUMP_CONTROL_TYPE_RIGHT ? \
                    LED_LEVEL_MAX / 2 : LED_LEVEL_MAX;
        msg.led_index = PUMP_RIGHT_CONTROL_BL;
        led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
    }
}

void airplane_mode_pump_policy(ui_proc_airplane_mode_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    if (proc->pump->is_on_working() > 0) {
        proc->pump->set_breast_pump_status(BREAST_PUMP_FINISH);
    }
}
