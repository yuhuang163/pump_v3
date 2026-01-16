/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_dummy.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define DUMMY_DISPLAY_TASK_NAME              "dummy_display_cmp"

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
static void dummy_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    LOG_UI("dummy play finish.\n");
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    uint8_t airplane_mode_state = 0;
    if (wm->is_current_window(wm, ID_WINDOW_DUMMY) == 0) {
        return;
    }
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_LOGO;
    wm->dispatch_new_window_args(wm, &msg);
    return;
}

static void dummy_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void dummy_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_dummy_process(ui);
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= DUMMY_ANIMATION_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(dummy_display_completed_cb, proc, DUMMY_DISPLAY_TASK_NAME);
    }
}

static void dummy_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            dummy_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            dummy_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void dummy_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_dummy_prepare(ui);
}

static void dummy_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_dummy_remove(ui);
}

static void dummy_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(DUMMY_DISPLAY_TASK_NAME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int switch_dummy_night_light_onoff_ui(ui_proc_dummy_t *proc)
{
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
    LOG_UI("switch dummy night light power: %d, level: %d", night_light_info.onoff, night_light_info.level);

    return 0;
}

void start_dummy_ui_animation(ui_proc_dummy_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == DUMMY_UI_TYPE_HOME) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "dummy_screen",
        .fps = DUMMY_ANIMATION_FREQ,
        .progress_cb = dummy_animation_progress_cb,
        .display_prepare_cb = dummy_ui_display_prepare,
        .display_remove_cb = dummy_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = DUMMY_UI_TYPE_HOME;
}

void stop_dummy_ui_animation(ui_proc_dummy_t *proc)
{
    dummy_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}