/*
* INCLUDE FILES
*****************************************************************************************
*/
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
static void on_ble_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{

}

static int on_post_ble_init_route(ble_manager_context_t *ctx)
{
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    ADD_BATTERY_STATE_CB(battery, ctx, on_ble_battery_state_change_cb);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_ble_hook(ble_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __ble_manager_hook));
    ctx->hook->on_post_init_route = on_post_ble_init_route;
}

