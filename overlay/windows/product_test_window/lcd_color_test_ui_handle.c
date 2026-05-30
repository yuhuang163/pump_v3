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
#define GOTO_LCD_COLOR_TEST_TASK_NAME                  "goto_lcd_color_test"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void lcd_color_test_ui_after_policy(ui_manager_context_t *ui, int index)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)ui->priv.play_info.user_data;
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
    ui_display_lcd_color_test_process(ui);
}

static void lcd_color_test_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            lcd_color_test_ui_after_policy(ui, index);
            break;
        case ANI_POPUP_DISPLAY_FINISH:
            break;
        default:
            break;
    }
}

static void lcd_color_test_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_lcd_color_test_prepare(ui);
}

static void lcd_color_test_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_lcd_color_test_remove(ui);
}

static void goto_lcd_color_test_policy(hdr_post_args_t *args)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    show_lcd_color_test_ui(proc);
}

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
void show_lcd_color_test_ui(ui_proc_product_test_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    proc->last_progress_ani_time = get_timestamp();
    if (proc->current_ui_type == PRODUCT_TEST_UI_TYPE_LCD_COLOR) {
        return;
    }

    ui_play_info_t play_info = {
        .tag = "lcd_color_test_screen",
        .fps = PRODUCT_TEST_ANIMATION_FREQ,
        .progress_cb = lcd_color_test_animation_progress_cb,
        .display_prepare_cb = lcd_color_test_ui_display_prepare,
        .display_remove_cb = lcd_color_test_ui_display_remove,
        .user_data = proc,
    };
    // proc->lcd_color = 0xFFFFFF;
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = PRODUCT_TEST_UI_TYPE_LCD_COLOR;
}

void lcd_color_test_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(GOTO_LCD_COLOR_TEST_TASK_NAME);
}

void lcd_color_test_keyevent_cb(ui_proc_product_test_t *proc, uint16_t keycode, uint16_t keytype)
{
    switch (keycode) {
        default:
        case KEY_CODE_POWER:
        case KEY_CODE_MODE:
#if IS_SUPPORT_FREQUENCY_KEY
        case KEY_CODE_FREQUENCY:
#endif
#if IS_SUPPORT_CANCEL_KEY
        case KEY_CODE_CANCEL:
#endif
        case KEY_CODE_BREASTFEEDING:
            return;
        case KEY_CODE_START:
            proc->lcd_color = 0x00FF00;
            break;
        case KEY_CODE_LEFT_CONTROL:
            proc->lcd_color = 0xFF0000;
            break;
        case KEY_CODE_RIGHT_CONTROL:
            proc->lcd_color = 0x0000FF;
            break;
    }

    if (proc->current_ui_type != PRODUCT_TEST_UI_TYPE_LCD_COLOR) {
        POST_WM_TASK_HANDLER_NO_DELAY(goto_lcd_color_test_policy, proc, GOTO_LCD_COLOR_TEST_TASK_NAME);
    }
}