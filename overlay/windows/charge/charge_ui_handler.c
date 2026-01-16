/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

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
static void charging_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void charging_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_charge_screen(ui);
}

static void charging_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            charging_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            charging_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void charging_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_charge_screen_prepare(ui);
}

static void charging_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_charge_screen_remove(ui);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_charging_ui(ui_proc_charge_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == CHARGE_UI_TYPE_CHARGING) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "charge_screen",
        .fps = CHARGE_ANIMATION_FREQ,
        .progress_cb = charging_animation_progress_cb,
        .display_prepare_cb = charging_ui_display_prepare,
        .display_remove_cb = charging_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
}

void stop_charging_ui(ui_proc_charge_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}

int switch_charge_night_light_onoff_ui(ui_proc_charge_t *proc)
{
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->st);
    EXCEPTION_CHECK_INT(proc->pbus);
    struct rt_device_node *led_dev = proc->pbus->find_device(ID_DEVICE_LED);
    EXCEPTION_CHECK_INT(led_dev);
    night_light_info_t night_light_info = {0};
    proc->st->get_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_INFO, 0, 0, &night_light_info);
    if (night_light_info.adaptive) {
        LOG_UI("night light is adaptive, ignore.");
        return 0;
    }
    night_light_info.onoff = !night_light_info.onoff;
    if (night_light_info.level == 0) {
        night_light_info.level = 5;
    } 
    proc->st->put_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_INFO, 0, 0, &night_light_info);
    led_status_msg_t msg = {
        .led_index = NIGHT_LIGHT_LED_ID_0,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous.level = night_light_info.onoff ? night_light_info.level : 0,
        },
    };
    led_dev->ioctl(led_dev, OP_CODE_NIGHT_LIGHT_STATUS_SET, (void *)&msg);
    LOG_UI("switch charge night light power: %d, level: %d", night_light_info.onoff, night_light_info.level);

    return 0;
}
