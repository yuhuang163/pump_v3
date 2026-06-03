/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h"
#include "ble_manager.h" // IWYU pragma: keep
#include "ble_data_bury_message.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int ble_data_bury_srv_recv_data(void *context, ble_service_data_t *srv_data)
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
    LOG_BLE("ble_data_bury_recv_data[%d]", srv_data->size);
    LOG_HEX("data_bury_rx", srv_data->p_data, srv_data->size);
    ctx->dispatch_ble_client(ctx, DEVICE_BLE_DATA_BURY_MSG_CHANNEL, srv_data->p_data, srv_data->size);
    return 0;
}

int ble_data_bury_srv_send_data(void *context, ble_service_data_t *srv_data)
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
