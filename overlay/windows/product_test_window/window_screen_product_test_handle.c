/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_product_test.h"
#include "factory_tests_handler.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                    product_test
#define PAGE_NAME_STR                       "product_test"
#define PRODUCT_TEST_UI_KEY_CODE            (KEY_CODE_POWER |KEY_CODE_START | KEY_CODE_MODE | KEY_CODE_FREQUENCY | \
                                            KEY_CODE_BREASTFEEDING | KEY_CODE_LEFT_CONTROL | KEY_CODE_RIGHT_CONTROL)
#define PRODUCT_TEST_UI_KEY_TYPE            (KEY_EVENT_CLICK)
#define PRODUCT_TEST_START_TASK_NAME        "product_test_start_ui"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_START_TASK_NAME); \
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

static void factory_tests_event_report_cb(void *user_data, uint16_t event_id, void *data, int data_len)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return;
    }
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)user_data;
    EXCEPTION_CHECK_VOID(proc);
    LOG_WM("factory_tests_event_report_cb event_id: %d", event_id);
    switch (event_id) {
        case FAC_TST_READ_SW_VER_EVENT: {
            EXCEPTION_CHECK_VOID(data);
            char *sw_ver = (char *)data;
            LOG_WM("factory test sw ver: %s", sw_ver);
        }
        break;
        default:
            break;
    }
}

static void product_test_ui_common_cb(hdr_post_args_t *args)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return;
    }
    ui_proc_product_test_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    int action = args->arg1;
    switch (action) {
        case PRODUCT_TEST_START: {
            window_manager_context_t *wm = proc->wm;
            EXCEPTION_CHECK_VOID(wm && wm->priv.product_tst);
            LOG_WM("%s", args->name);
            char buffer[64] = {0};
            wm->priv.product_tst->add_product_tests_evt_lisenser(proc, TST_FACTORY_TYPE, factory_tests_event_report_cb);
            wm->priv.product_tst->tests_manager_set_enable(true);
            wm->priv.product_tst->get_factory_tests_event_process(FAC_TST_READ_SW_VER_EVENT, 0, 0, &buffer[0]);
        }
        break;
        case PRODUCT_TEST_E10: {
            LOG_WM("%s", args->name);
        }
        break;
        case PRODUCT_TEST_E11: {
            LOG_WM("%s", args->name);
        }
        break;
        default:
        break;
    }

}

static int product_test_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return 0;
    }
    ui_proc_product_test_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    key_input_context_t *key_ctx = proc->wm->priv.key;
    EXCEPTION_CHECK_INT(key_ctx);
    const char *key_name = NULL;
    if (key_ctx && key_ctx->hook && key_ctx->hook->get_key_code_name) {
        key_name = key_ctx->hook->get_key_code_name(keycode);
    } else {
        key_name = "@unknow";
    }
    LOG_WM("%s, keyevent#ENTER: %d/%d.", key_name, keycode, keytype);
    switch (keycode) {
        case KEY_CODE_POWER:
            break;
        case KEY_CODE_START:
            break;
        case KEY_CODE_MODE:
            break;
        case KEY_CODE_FREQUENCY:
            break;
        case KEY_CODE_BREASTFEEDING:
            break;
        case KEY_CODE_LEFT_CONTROL:
            break;
        case KEY_CODE_RIGHT_CONTROL:
            break;
        default:
            LOG_WM("unknow keycode: %d, ignore.", keycode);
            break;
    }
    LOG_WM("keyevent, EXIT: %d: %d", keycode, keytype);

    return 0;
}

static void product_test_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
     broadcast_priv_data_t *pdata)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, return.");
        return;
    }

    switch (id.code) {
        case ID_BROADCAST_FACTORY_TEST_CMD: {
            break;
        }
        
        case ID_BROADCAST_SYS_PERSIST_INFO_SAVE: {
        }
        break;

        default:
        break;
    }
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_product_test_t *proc = CONTAINER_OF(thiz, ui_proc_product_test_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_INT(ui);
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        if (wm->priv.key) {
            wm->priv.key->add_keyevent_handler("product_test_wm", proc, PRODUCT_TEST_UI_KEY_CODE, 
                PRODUCT_TEST_UI_KEY_TYPE, product_test_ui_keyevent_hdr);
        }
        POST_TASK_HANDLER_DELAYED_ARG2(product_test_ui_common_cb, 100, PRODUCT_TEST_START, proc,
            PRODUCT_TEST_START_TASK_NAME);
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>PRODUCT_TEST_ON_CREATE");
        ui->animation_play_status_update(ui, false); /// 清除动画播放标志
        show_product_tests_ui(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_product_test_t *proc = CONTAINER_OF(thiz, ui_proc_product_test_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE: {
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_product_tests_ui_animation(proc);
            REMOVE_ALL_HANDER_TASK();
            unregister_broadcast_receiver(&proc->receiver);
            if (wm->priv.product_tst) {
                wm->priv.product_tst->remove_product_tests_evt_lisenser(factory_tests_event_report_cb);
            } else {
                LOG_WM("product_tst not present, ignore.");
            }
            if (wm->priv.key) {
                wm->priv.key->remove_keyevent_handler(product_test_ui_keyevent_hdr);
            } else {
                LOG_WM("key not present, ignore.");
            }
        }
        break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            u_free(proc);
            proc = NULL;
            LOG_WM("==>ON_DESTORY end");
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
    LOG_WM("enter, time: %ld.", get_timestamp());
    ui_proc_product_test_t *proc = u_malloz(sizeof(ui_proc_product_test_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    /// 设置窗口标识.
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "product_test_brc";
    proc->receiver.id.code |= ID_BROADCAST_FACTORY_TEST_CMD;
    proc->receiver.id.code |= ID_BROADCAST_SYS_PERSIST_INFO_SAVE;
    proc->receiver.receiver_cb = product_test_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);
    /// 设置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1; 
    wm->dispatch_new_window(w_ctx);
    return ID_WINDOW_DISPLAY_OK;
}

