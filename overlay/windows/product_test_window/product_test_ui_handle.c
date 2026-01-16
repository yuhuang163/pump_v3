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
#define PRODUCT_TESTS_HDR_TASK_NAME                  "product_tests_display_cmp"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void product_tests_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->st);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE) == 0) {
        return;
    }
    multi_func_msg_data_t msg = {0};
    uint8_t guide_deploy_flag = 0;
    proc->st->get_store_ram_val(STORE_ID_SYS_GUIDE_DEPLOYED, 0, 0, &guide_deploy_flag);
    if(guide_deploy_flag == 0) {
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_HOME;
        wm->dispatch_new_window_args(wm, &msg);
        return;
    }
    LOG_WM("goto pump screen");
    msg.union_body.extra.window_id = ID_WINDOW_PUMP;
    wm->dispatch_new_window_args(wm, &msg);
}

static void product_tests_ui_before_policy(ui_manager_context_t *ui, int index)
{

}

static void product_tests_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_display_product_tests_home_process(ui);
#if 0
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PRODUCT_TESTS_ANIMATION_DISPLAY_TIME) {
        POST_TASK_HANDLER_NO_DELAY(product_tests_display_completed_cb, proc,
            PRODUCT_TESTS_HDR_TASK_NAME);
    }
#endif
}

static void product_tests_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            product_tests_ui_before_policy(ui, index);
            break;
        case ANI_TIMER_DRAW_AFTER:
            product_tests_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void product_tests_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TESTS_HDR_TASK_NAME);
}

static void product_tests_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_product_tests_home_prepare(ui);
}

static void product_tests_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_product_tests_home_remove(ui);
}
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
void show_product_tests_ui(ui_proc_product_test_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == PRODUCT_TEST_UI_TYPE_HOME) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }

    ui_play_info_t play_info = {
        .tag = "product_tests_screen",
        .fps = PRODUCT_TEST_ANIMATION_FREQ,
        .progress_cb = product_tests_animation_progress_cb,
        .display_prepare_cb = product_tests_ui_display_prepare,
        .display_remove_cb = product_tests_ui_display_remove,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PRODUCT_TEST_UI_TYPE_HOME;
}

void stop_product_tests_ui_animation(ui_proc_product_test_t *proc)
{
    product_tests_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}

void product_tests_start_key_event_handler(ui_proc_product_test_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->tests);
    uint8_t current_mode = TST_NONE_TYPE;
    if (proc->tests->tests_manager_get_current_mode) {
        current_mode = proc->tests->tests_manager_get_current_mode();
    }

    if(current_mode != TST_NONE_TYPE) {
        return;
    }
    POST_TASK_HANDLER_NO_DELAY(product_tests_display_completed_cb, proc,
        PRODUCT_TESTS_HDR_TASK_NAME);

}