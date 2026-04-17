/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h"
#include "root_common_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

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
static int on_airplane_mode_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    return 0;
}

static int on_count_down_popup_window_exit_cb(void *user_data, int args1, int args2)
{
    airplane_mode_context_t *ctx = user_data;
    EXCEPTION_CHECK_INT(ctx);
    ctx->priv.is_count_down_popup_show = false;
    ctx->enter(AIRPLANE_MODE_BY_USER_ACTIVE_ENTER);
    LOG_AIR("on_count_down_popup_window_exit_cb, goto airplane mode.");
    return 0;
}

static int airplane_mode_key_event_handler(void *user_data, uint16_t keycode, uint16_t keytype)
{
    LOG_AIR("airplane_mode_key_event_handler: %d, %d", keycode, keytype);
    airplane_mode_context_t *ctx = user_data;
    EXCEPTION_CHECK_INT(ctx);
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(wm);
    if (keytype == KEY_EVENT_UP) {
        if (ctx->priv.is_count_down_popup_show) {
            wm->close_popup_overlay(wm);
            ctx->priv.is_count_down_popup_show = false;
        }
    } else if (keytype == KEY_EVENT_DOWN && wm->is_current_window(wm, ID_WINDOW_AIRPLANE_MODE) == 0) {
        battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
        if (battery && battery->is_adapter_mismatch() > 0) {
            LOG_AIR("enter airplane mode, but adapter mismatch, ignore.");
            return 0;
        }
        ctx->priv.is_count_down_popup_show = true;
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_POPUP_WINDOW;
        msg.union_body.extra.is_bg_dispatch = 0x01;
        msg.union_body.extra.arg1 = ID_POPUP_UI_COUNT_DOWN;
        msg.union_body.extra.popup_exit_time = 5000;
        msg.union_body.extra.user_data = ctx;
        msg.union_body.extra.popup_exit_cb = on_count_down_popup_window_exit_cb;
        wm->dispatch_new_window_args(wm, &msg);
    }
    return 0;
}

static int on_post_airplane_mode_init_route(struct airplane_mode_context *ctx)
{
    key_input_context_t *input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    if (input) {
        input->add_keyevent_handler("airplane_mode", ctx, KEY_CODE_TRAVEL_LOCK, KEY_EVENT_DOWN | KEY_EVENT_UP, 
            airplane_mode_key_event_handler, KEY_HANDLER_SHIELD_LEVEL_1);
    }
    return 0;
}

static int enter_airplane_mode_route(struct airplane_mode_context *ctx, int enter_reason)
{
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        st->put_store_ram_val(STORE_ID_SYS_AIRPLANE_MODE_STATE, 1, 0, NULL);
    }
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(wm);
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
    wm->dispatch_new_window_args(wm, &msg);

    ///通知app层进入飞行模式
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_INT(servers_ctx);
    servers_publish_data_args_t args_temp = {
        .arg1 = OLD_PUBLISH_ABNORMAL_TRAVEL_LOCK,
        .arg2 = OLD_PUBLISH_ABNORMAL_STATE_ACTIVE,
        .val = NULL,
    };
    servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, &args_temp);
    return 0;
}

static int exit_airplane_mode_route(struct airplane_mode_context *ctx, int exit_reason)
{
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        st->put_store_ram_val(STORE_ID_SYS_AIRPLANE_MODE_STATE, 0, 0, NULL);
    }

    ///通知app层退出飞行模式
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_INT(servers_ctx);
    servers_publish_data_args_t args_temp = {
        .arg1 = OLD_PUBLISH_ABNORMAL_TRAVEL_LOCK,
        .arg2 = OLD_PUBLISH_ABNORMAL_STATE_CLEARED,
        .val = NULL,
    };
    servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL, &args_temp);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_airplane_mode_hook(airplane_mode_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __airplane_mode_hook));
    ctx->hook->on_post_init_route = on_post_airplane_mode_init_route;
    ctx->hook->on_airplane_mode_receiver_route = on_airplane_mode_receiver_route;
    ctx->hook->enter_airplane_mode_route = enter_airplane_mode_route;
    ctx->hook->exit_airplane_mode_route = exit_airplane_mode_route;
}
