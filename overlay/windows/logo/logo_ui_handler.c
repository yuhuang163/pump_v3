/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_logo.h" // IWYU pragma: keep
#include "ui_img_define.h"

/*
* DEFINES
*****************************************************************************************
*/
#define LOGO_ANIMATION_FREQ                 (12)
#define LOGO_DISPLAY_TASK_NAME              "logo_display_cmp"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/// 泵奶首页动画资源ID.
static const uint16_t s_logo_screen_res_array[1] = {
    UI_IMG_ID_LOGO,
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void logo_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_logo_t *proc = (ui_proc_logo_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    LOG_UI("logo play finish.\n");
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    uint8_t airplane_mode_state = 0;
    if (wm->is_current_window(wm, ID_WINDOW_LOGO) == 0) {
        return;
    }
    st->get_store_ram_val(STORE_ID_SYS_AIRPLANE_MODE_STATE, 0, 0, &airplane_mode_state);
    LOG_UI("airplane_mode_state: %d", airplane_mode_state);
    if (airplane_mode_state == 1) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
        wm->dispatch_new_window_args(wm, &msg);
        return;
    }
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_PUMP;
    wm->dispatch_new_window_args(wm, &msg);
}

static void logo_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_logo_t *proc = (ui_proc_logo_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= LOGO_ANIMATION_DISPLAY_TIME) {
        POST_TASK_HANDLER_DELAYED_ARG1(logo_display_completed_cb, 10, proc,
            LOGO_DISPLAY_TASK_NAME);
    }
}

static void logo_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            logo_ui_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            break;
        default:
            break;
    }
}

static void show_logo_ui(ui_proc_logo_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "logo_screen",
        .fps = LOGO_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_logo_screen_res_array,
        .res_count = LENGTH_OF_ARRAY(s_logo_screen_res_array),
        .progress_cb = logo_animation_progress_cb,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
}

static void logo_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(LOGO_DISPLAY_TASK_NAME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_logo_ui_animation(ui_proc_logo_t *proc)
{
    power_manager_context_t *pm = proc->wm->priv.pm;
    EXCEPTION_CHECK_VOID(pm);
    while (get_timestamp_ms(0) < BOOT_LONG_PRESS_TIME) {
        if (pm->is_system_power_on()) {
            break;
        }
        msleep(10);
    }
    LOG_UI("start logo ui animation.");
    show_logo_ui(proc);
}

void stop_logo_ui_animation(ui_proc_logo_t *proc)
{
    logo_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = NULL,
        .fps = LOGO_ANIMATION_FREQ,
        .res_array = NULL,
        .res_count = 0,
        .progress_cb = NULL,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}