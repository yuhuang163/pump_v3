/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_logo.h" // IWYU pragma: keep
#include "ui_img_define.h"
#include "window_screen_product_test.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PRODUCT_TESTS_ANIMATION_FREQ                 (12)
#define PRODUCT_TESTS_HDR_TASK_NAME                  "product_tests_display_cmp"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
///产测动画资源ID.
static const uint16_t s_product_tests_screen_res_array[1] = {
    UI_IMG_ID_PRODUCT_TESTS_RESOUCE,
};

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void product_tests_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_logo_t *proc = (ui_proc_logo_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_LOGO) == 0) {
        return;
    }
}

static void product_tests_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_logo_t *proc = (ui_proc_logo_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PRODUCT_TESTS_ANIMATION_DISPLAY_TIME) {
        POST_TASK_HANDLER_DELAYED_ARG1(product_tests_display_completed_cb, 10, proc,
            PRODUCT_TESTS_HDR_TASK_NAME);
    }
}

static void product_tests_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TESTS_HDR_TASK_NAME);
}

static void product_tests_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            product_tests_ui_policy(ui);
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
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
void show_product_tests_ui(ui_proc_product_test_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "product_tests_screen",
        .fps = PRODUCT_TESTS_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_product_tests_screen_res_array,
        .res_count = LENGTH_OF_ARRAY(s_product_tests_screen_res_array),
        .progress_cb = product_tests_animation_progress_cb,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
}

void stop_product_tests_ui_animation(ui_proc_product_test_t *proc)
{
    product_tests_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = NULL,
        .fps = PRODUCT_TESTS_ANIMATION_FREQ,
        .res_array = NULL,
        .res_count = 0,
        .progress_cb = NULL,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    ui->animation_play_status_update(ui, false);
    proc->last_progress_ani_time = get_timestamp();
}
