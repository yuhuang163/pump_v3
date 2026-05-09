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
    ble_manager_context_t *ble = (ble_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(ble);
    if (info->battery_state == BATTERY_STATE_EMPTY || 
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN ||
        info->battery_state == BATTERY_STATE_FORBID_START ||
        info->battery_state == BATTERY_STATE_LOW_POWER_RUN) {
        if (ble->get_adv_state() || ble->get_adv_speed() != BLE_OPT_ADV_STOP) {
            ble->bt_adv_stop_by_user();
        }
    } else {
        if (!ble->get_adv_state()) {
            ble->bt_adv_start_by_user();
        }
    }
}

static int on_post_ble_init_route(ble_manager_context_t *ctx)
{
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    if (battery) {
        battery->add_battery_state_cb(ctx, on_ble_battery_state_change_cb);
    }
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

