/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h"
#include "ble_manager.h" // IWYU pragma: keep
#include "ble_master_message.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ble_protocol_list_t ble_master_msg_process_list[] = {
    {BLE_SIID_BIND_CONFIG,          NULL},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void user_bind_succ_send(hdr_post_args_t *args)
{
    ble_manager_context_t *p_ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    p_ble->dispatch_ble_event_wrapper(p_ble, BLE_MSG_USER_BIND_SUCC, args->user_data, sizeof(user_bind_info_t));
}

static int ble_master_msg_battery_handle(void *context, void *message)
{
    LOG_BLE("BLE_SIID_BATTERY_REQ");
    return 0;
}

static int ble_master_msg_language_handle(void *context, void *message)
{
    return 0;
}
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int ble_master_srv_recv_data(void *context, ble_service_data_t *srv_data)
{
    ble_manager_context_t *ctx = (ble_manager_context_t *)context;
    EXCEPTION_CHECK_INT(ctx);
    EXCEPTION_CHECK_INT(srv_data);
    if (!srv_data->p_data || srv_data->size == 0) {
        LOGE("recv data is NULL or size is 0");
        return -1;
    }
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    if (pm && (pm->is_pm_suspend() || pm->is_pm_idle())) {
        pm->pm_wakeup(PM_WAKEUP_REASON_BLE_REQ);
    }
    LOG_BLE("ble_master_recv_data[%d]", srv_data->size);
    LOG_HEX("recv_data hex", srv_data->p_data, srv_data->size);
    ctx->dispatch_ble_client(ctx, DEVICE_BLE_MASTER_MSG_CHANNEL, srv_data->p_data, srv_data->size);
    return 0;
}

int ble_master_srv_send_data(void *context, ble_service_data_t *srv_data)
{
    ble_manager_context_t *ctx = (ble_manager_context_t *)context;
    EXCEPTION_CHECK_INT(ctx);
    EXCEPTION_CHECK_INT(srv_data);
    if (!ctx->ble_is_connected()) {
        LOGE("ble is not connected, ignore");
        return -2;
    }
    return 0;
}
