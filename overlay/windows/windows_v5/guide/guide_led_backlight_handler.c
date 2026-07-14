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

static void guide_led_backlight_level_page_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_return_only(proc);
    if (proc->led == NULL) {
        return;
    }

    switch (proc->page_index) {
        case GUIDE_PAGE_LEVEL_LEFT_GUIDE:
            proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, blink_on ? LED_LEVEL_MAX : 0);
            break;
        case GUIDE_PAGE_LEVEL_LEFT:
            proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, LED_LEVEL_MAX);
            break;
        case GUIDE_PAGE_LEVEL_RIGHT_GUIDE:
            proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, blink_on ? LED_LEVEL_MAX : 0);
            break;
        case GUIDE_PAGE_LEVEL_RIGHT:
            proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, LED_LEVEL_MAX);
            break;
        default:
            break;
    }
}

static void guide_led_backlight_mode_guide_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_return_only(proc);
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_MODE_BL, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_mode_select_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_return_only(proc);
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_MODE_BL, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_rhythm_guide_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_return_only(proc);
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_BREASTFEEDING_BL, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_rhythm_select_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_return_only(proc);
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_BREASTFEEDING_BL, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_pump_start_guide_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_return_only(proc);
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_START_BL, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_intro_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_return_only(proc);
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_START_BL, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_key_guide_update(ui_proc_guide_t *proc)
{
    guide_led_backlight_all_on(proc);
}

static void guide_led_backlight_pump_stop_guide_update(ui_proc_guide_t *proc)
{
    const bool blink_on = proc->state_blink_visible != 0;

    guide_led_backlight_all_off(proc);
    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(PUMP_FREQ_SET_BL, blink_on ? LED_LEVEL_MAX : 0);
}

static void guide_led_backlight_blink_led_set(ui_proc_guide_t *proc, uint8_t led_index)
{
    const bool blink_on = proc->state_blink_visible != 0;

    if (proc->led == NULL) {
        return;
    }
    proc->led->set_led_level(led_index, blink_on ? LED_LEVEL_MAX : 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_led_backlight_blink_tick(ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);

    switch (proc->page_index) {
        case GUIDE_PAGE_INTRO:
        case GUIDE_PAGE_FUNCTION_HOME:
        case GUIDE_PAGE_PUMP_START_GUIDE:
            guide_led_backlight_blink_led_set(proc, PUMP_START_BL);
            break;
        case GUIDE_PAGE_LEVEL_LEFT_GUIDE:
            guide_led_backlight_blink_led_set(proc, PUMP_LEFT_CONTROL_BL);
            break;
        case GUIDE_PAGE_LEVEL_RIGHT_GUIDE:
            guide_led_backlight_blink_led_set(proc, PUMP_RIGHT_CONTROL_BL);
            break;
        case GUIDE_PAGE_MODE_SELECT_GUIDE:
            guide_led_backlight_blink_led_set(proc, PUMP_MODE_BL);
            break;
        case GUIDE_PAGE_MODE_SELECT:
            guide_led_backlight_blink_led_set(proc, PUMP_MODE_BL);
            break;
        case GUIDE_PAGE_RHYTHM_SELECT_GUIDE:
            guide_led_backlight_blink_led_set(proc, PUMP_BREASTFEEDING_BL);
            break;
        case GUIDE_PAGE_RHYTHM_SELECT:
            guide_led_backlight_blink_led_set(proc, PUMP_BREASTFEEDING_BL);
            break;
        case GUIDE_PAGE_PUMP_STOP_GUIDE:
            guide_led_backlight_blink_led_set(proc, PUMP_FREQ_SET_BL);
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
            guide_led_backlight_all_on(proc);
            break;
        case GUIDE_PAGE_INTRO:
            guide_led_backlight_intro_update(proc);
            break;
        case GUIDE_PAGE_KEY_GUIDE:
            guide_led_backlight_key_guide_update(proc);
            break;
        case GUIDE_PAGE_KNOB_GUIDE:
            guide_led_backlight_return_only(proc);
            break;
        case GUIDE_PAGE_FUNCTION_HOME:
            guide_led_backlight_intro_update(proc);
            break;
        case GUIDE_PAGE_LEVEL_BOTH_GUIDE:
        case GUIDE_PAGE_LEVEL_BOTH:
            guide_led_backlight_return_only(proc);
            break;
        case GUIDE_PAGE_LEVEL_LEFT_GUIDE:
        case GUIDE_PAGE_LEVEL_LEFT:
        case GUIDE_PAGE_LEVEL_RIGHT_GUIDE:
        case GUIDE_PAGE_LEVEL_RIGHT:
            guide_led_backlight_level_page_update(proc);
            break;
        case GUIDE_PAGE_MODE_SELECT_GUIDE:
            guide_led_backlight_mode_guide_update(proc);
            break;
        case GUIDE_PAGE_MODE_SELECT:
            guide_led_backlight_mode_select_update(proc);
            break;
        case GUIDE_PAGE_RHYTHM_SELECT_GUIDE:
            guide_led_backlight_rhythm_guide_update(proc);
            break;
        case GUIDE_PAGE_RHYTHM_SELECT:
            guide_led_backlight_rhythm_select_update(proc);
            break;
        case GUIDE_PAGE_PUMP_START_GUIDE:
            guide_led_backlight_pump_start_guide_update(proc);
            break;
        case GUIDE_PAGE_PUMP_STOP_GUIDE:
            guide_led_backlight_pump_stop_guide_update(proc);
            break;
        case GUIDE_PAGE_FINISH:
            guide_led_backlight_all_on(proc);
            break;
        default:
            guide_led_backlight_return_only(proc);
            break;
    }
}
