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

static int airplane_mode_key_event_handler(void *user_data, uint16_t keycode, uint16_t keytype)
{
    LOG_AIR("airplane_mode_key_event_handler: %d, %d", keycode, keytype);
    airplane_mode_context_t *ctx = user_data;
    EXCEPTION_CHECK_INT(ctx);
    ctx->enter(AIRPLANE_MODE_BY_USER_ACTIVE_ENTER);
    return 0;
}

static int on_post_airplane_mode_init_route(struct airplane_mode_context *ctx)
{
    /// 注册按键唤醒.
    key_input_context_t *input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    if (input) {
        input->add_keyevent_handler("airplane_mode", ctx, KEY_CODE_TRAVEL_LOCK, KEY_EVENT_LONG_5S,  airplane_mode_key_event_handler);
    }

#if IS_SUPPORT_DEVICE_IMU
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    sensor->imu.add_irq_callback(IMU_IRQ_EVENT_CB_AIRPLANE, airplane, on_imu_irq_callback);
#endif
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
    return 0;
}

static int exit_airplane_mode_route(struct airplane_mode_context *ctx, int exit_reason)
{
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        st->put_store_ram_val(STORE_ID_SYS_AIRPLANE_MODE_STATE, 0, 0, NULL);
    }
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(wm);
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_PUMP;
    wm->dispatch_new_window_args(wm, &msg);
    
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
