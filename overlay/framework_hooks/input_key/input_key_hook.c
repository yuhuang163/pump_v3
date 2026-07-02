/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "input_key_code.h"  // IWYU pragma: keep
#include "key_input_manager.h" // IWYU pragma: keep

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
static int input_keyevent_route(struct key_input_context *ctx, uint16_t keycode, uint16_t keytype, int flag)
{
#if IS_SUPPORT_KEY_SOFT_RESET      
    if (keycode == SYS_SOFT_RESET_CODE && keytype == KEY_EVENT_LONG_5S) {
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        EXCEPTION_CHECK_INT(wm);
        if (wm->is_current_window(wm, ID_WINDOW_AIRPLANE_MODE) ||
            wm->is_current_window(wm, ID_WINDOW_FACTORY_RESET) || 
            wm->is_current_window(wm, ID_WINDOW_CHARGE) ||
            wm->is_current_window(wm, ID_WINDOW_POWER_OFF) ||
            wm->is_current_window(wm, ID_WINDOW_SYSTEM_ABNORMAL)) { 
            LOG_KEY("in airplane mode or factory reset or charge window or abnormal window, ignore");
            return 0;
        }
        breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
        if (pump) {
            int is_working = 0;
            READ_PUMP_IS_WORKING(pump, is_working);
            if (is_working) {
                LOG_KEY("in pump, ignore");
                return 0;
            }
        }
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_FACTORY_RESET;
        msg.union_body.extra.arg1 = 1; /// 1: factory reset confirm
        wm->dispatch_new_window_args(wm, &msg);
        LOG_KEY("goto factory reset");
    }
#endif 
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_input_key_hook(key_input_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __key_input_hook));
    ctx->hook->input_keyevent_route = input_keyevent_route;
    ctx->hook->get_key_code_name = get_input_key_code_name;
}
