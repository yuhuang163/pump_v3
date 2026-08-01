/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_guide.h"
#include "hal_led.h" // IWYU pragma: keep

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_led_backlight_all_off(ui_proc_guide_t *proc)
{
    if (proc->led == NULL) {
        return;
    }

    for (uint8_t i = 0; i < PUMP_LED_INDEX_MAX; i++) {
        proc->led->set_led_level(i, 0);
    }
    if (proc->led->hook != NULL && proc->led->hook->set_key_backlight != NULL) {
        proc->led->hook->set_key_backlight(proc->led, 0);
    }
}

static void guide_led_backlight_all_on(ui_proc_guide_t *proc)
{
    if (proc->led == NULL) {
        return;
    }

    for (uint8_t i = 0; i < PUMP_LED_INDEX_MAX; i++) {
        proc->led->set_led_level(i, LED_LEVEL_MAX);
    }
}

static void guide_led_backlight_non_return_off(ui_proc_guide_t *proc)
{
    if (proc->led == NULL) {
        return;
    }

    for (uint8_t i = 0; i < PUMP_LED_INDEX_MAX; i++) {
        if (i != PUMP_FREQ_SET_BL) {
            proc->led->set_led_level(i, 0);
        }
    }
}

static void guide_led_backlight_return_only(ui_proc_guide_t *proc)
{
    guide_led_backlight_non_return_off(proc);
    if (proc->led != NULL) {
        proc->led->set_led_level(PUMP_FREQ_SET_BL, LED_LEVEL_MAX);
    }
}

static void guide_led_backlight_lr_set(ui_proc_guide_t *proc, uint8_t level)
{
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, level);
    proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, level);
}

static void guide_led_backlight_level_side_guide_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_all_off(proc);
    guide_led_backlight_lr_set(proc, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_level_side_update(ui_proc_guide_t *proc)
{
    guide_led_backlight_all_off(proc);
    guide_led_backlight_lr_set(proc, LED_LEVEL_MAX);
}

static void guide_led_backlight_speed_set(ui_proc_guide_t *proc, uint8_t level)
{
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_FREQ_SET_BL, level);
}

static void guide_led_backlight_speed_guide_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_all_off(proc);
    guide_led_backlight_speed_set(proc, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_speed_update(ui_proc_guide_t *proc)
{
    guide_led_backlight_all_off(proc);
    guide_led_backlight_speed_set(proc, LED_LEVEL_MAX);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_led_backlight_blink_tick(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_SIDE_GUIDE:
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
            guide_led_backlight_lr_set(proc,
                proc->state_blink_visible ? LED_LEVEL_MAX : 0);
            break;
        case GUIDE_PAGE_SPEED_GUIDE:
            guide_led_backlight_speed_set(proc,
                proc->state_blink_visible ? LED_LEVEL_MAX : 0);
            break;
        default:
            break;
    }
}

void guide_led_backlight_update(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);

    switch (proc->page_index) {
        case GUIDE_PAGE_WELCOME_ANIM:
        case GUIDE_PAGE_FINISH:
            guide_led_backlight_all_on(proc);
            break;
        case GUIDE_PAGE_QRCODE:
        case GUIDE_PAGE_INTRO:
        case GUIDE_PAGE_PUMP_START_GUIDE:
        case GUIDE_PAGE_PUMP_HOME_GUIDE:
        case GUIDE_PAGE_PUMP_PAUSE_GUIDE:
        case GUIDE_PAGE_PUMP_PAUSE:
        case GUIDE_PAGE_PUMP_FINISH_GUIDE:
        case GUIDE_PAGE_PUMP_FINISH_HOLD:
        case GUIDE_PAGE_PUMP_HOME:
        case GUIDE_PAGE_MODE_SWITCH_GUIDE:
        case GUIDE_PAGE_MODE_SWITCH:
        case GUIDE_PAGE_MODE_HOME:
        case GUIDE_PAGE_LEVEL_SIDE_DONE:
        case GUIDE_PAGE_LEVEL_BOTH_DONE:
        case GUIDE_PAGE_SPEED_DONE:
            guide_led_backlight_all_off(proc);
            break;
        case GUIDE_PAGE_LEVEL_SIDE_GUIDE:
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
            guide_led_backlight_level_side_guide_update(proc);
            break;
        case GUIDE_PAGE_LEVEL_SIDE:
        case GUIDE_PAGE_LEVEL_BOTH:
            guide_led_backlight_level_side_update(proc);
            break;
        case GUIDE_PAGE_SPEED_GUIDE:
            guide_led_backlight_speed_guide_update(proc);
            break;
        case GUIDE_PAGE_SPEED:
            guide_led_backlight_speed_update(proc);
            break;
        default:
            guide_led_backlight_return_only(proc);
            break;
    }
}
