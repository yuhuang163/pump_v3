/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_logo.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            logo
#define PAGE_NAME_STR                               "logo"
#define LOGO_UI_KEY_CODE                            (KEY_CODE_KNOB_RIGHT | KEY_CODE_KNOB_LEFT)
#define LOGO_UI_KEY_TYPE                            KEY_EVENT_CLICK
#define TOUCH_KEY_FW_VERSION_CHECK_TASK_NAME       "touch_key_fw_version_check"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(TOUCH_KEY_FW_VERSION_CHECK_TASK_NAME); \
} while (0)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
INIT_WINDOW_CONTEXT();

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void touch_key_fw_version_check_cb(hdr_post_args_t *args)
{
    ui_proc_logo_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_LOGO);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *key_dev = pbus->find_device(ID_DEVICE_INPUT_KEY);
    DEVICE_IOCTL_ARGS(key_dev , OP_CODE_INPUT_KEY_FW_VERSION_CHECK, NULL);
    LOG_WM("touch key fw version check");
}

#if IS_SUPPORT_LOGO_BG_COLOR_TEST
#if IS_SUPPORT_LOGO_KEY_LIGHT_ADAPTIVE_TEST
static void logo_test_key_backlight_on(ui_proc_logo_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
        proc->led->hook->set_key_backlight(proc->led, 100);
    }
}
#endif

static int on_logo_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    (void)keytype;
    ui_proc_logo_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_LOGO, 0);
    if (keycode == KEY_CODE_KNOB_LEFT) {
        proc->test_page_count--;
    } else if (keycode == KEY_CODE_KNOB_RIGHT) {
        proc->test_page_count++;
    }
#if IS_SUPPORT_LOGO_KEY_LIGHT_ADAPTIVE_TEST
    logo_test_key_backlight_on(proc);
#endif
    return 0;
}
#endif

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_logo_t *proc = CONTAINER_OF(thiz, ui_proc_logo_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
#if IS_SUPPORT_LOGO_BG_COLOR_TEST
        if (proc->key) {
            ADD_KEYEVENT_CB(proc->key, "home_wm", proc, LOGO_UI_KEY_CODE, LOGO_UI_KEY_TYPE,
                on_logo_ui_keyevent_hdr);
#if IS_SUPPORT_LOGO_KEY_LIGHT_ADAPTIVE_TEST
        logo_test_key_backlight_on(proc);
#endif
#endif
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        start_logo_ui_animation(proc);
        POST_TASK_HANDLER_DELAYED_ARG1(touch_key_fw_version_check_cb, 500, 
            proc, TOUCH_KEY_FW_VERSION_CHECK_TASK_NAME);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_logo_t *proc = CONTAINER_OF(thiz, ui_proc_logo_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_logo_ui_animation(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ALL_HANDER_TASK();
#if IS_SUPPORT_LOGO_BG_COLOR_TEST
            REMOVE_KEYEVENT_CB(proc->key, on_logo_ui_keyevent_hdr);
#endif
            u_free(proc);
            proc = NULL;
            LOG_WM("%s window exit--end", PAGE_NAME_STR);
            break;
        default:
            break;
    }

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
window_enter_result_e PAGE_HANDLE_NAME(PAGE_STATIC_NAME)(window_manager_context_t *wm, uint8_t window_id)
{
    LOG_WM("%s window enter, time: %ld.", PAGE_NAME_STR, get_timestamp());
    ui_proc_logo_t *proc = u_malloz(sizeof(ui_proc_logo_t));
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = LOGO_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}

