/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_product_test.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PRODUCT_HOME_TEST_HDR_TASK_NAME                  "product_home_test"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void product_home_test_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)args->user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
    multi_func_msg_data_t msg = {0};    
    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        LOG_WM("vbus plug in, goto charge screen");
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->battery && proc->battery->is_low_battery()) {
        LOG_WM("battery is low power, goto battery screen");
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void product_home_test_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
    ui_display_product_tests_home_process(ui);
#if 0
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    uint32_t time_diff = get_timestamp_ms(proc->last_progress_ani_time);
    if (time_diff >= PRODUCT_HOME_TEST_ANIMATION_DISPLAY_TIME) {
        proc->last_progress_ani_time = get_timestamp();
        LOG_UI("product home test ui timeout: %d", time_diff);
        POST_WM_TASK_HANDLER_NO_DELAY(product_home_test_display_completed_cb, proc,
            PRODUCT_HOME_TEST_HDR_TASK_NAME);
    }
#endif
}

static void product_home_test_ui_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            product_home_test_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void product_home_test_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_product_tests_home_prepare(ui);
}

static void product_home_test_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_product_tests_home_remove(ui);
}

static void product_home_test_start_key_event_handler(ui_proc_product_test_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->tests);
    uint8_t current_mode = TST_NONE_TYPE;
    if (proc->tests->tests_manager_get_current_mode) {
        current_mode = proc->tests->tests_manager_get_current_mode();
    }

    if (current_mode != TST_NONE_TYPE) {
        return;
    }
    POST_WM_TASK_HANDLER_NO_DELAY(product_home_test_display_completed_cb, proc,
        PRODUCT_HOME_TEST_HDR_TASK_NAME);
}

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
void product_home_test_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_HOME_TEST_HDR_TASK_NAME);
}

void show_product_home_test_ui(ui_proc_product_test_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == PRODUCT_TEST_UI_TYPE_HOME) {
        return;
    }

    ui_play_info_t play_info = {
        .tag = "product_home_test_screen",
        .fps = PRODUCT_TEST_ANIMATION_FREQ,
        .progress_cb = product_home_test_ui_progress_cb,
        .display_prepare_cb = product_home_test_ui_display_prepare,
        .display_remove_cb = product_home_test_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PRODUCT_TEST_UI_TYPE_HOME;
}

void product_home_test_keyevent_cb(ui_proc_product_test_t *proc, uint16_t keycode, uint16_t keytype)
{
    switch (keycode) {
        case KEY_CODE_START:
            product_home_test_start_key_event_handler(proc);
            break;
        default:
            LOG_WM("unknow keycode: %d, ignore.", keycode);
            break;
    }
}