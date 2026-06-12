/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ble_manager.h" // IWYU pragma: keep
#include "ble_factory_shell.h" // IWYU pragma: keep
#include "ble_factory_message.h"

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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int ble_factory_srv_recv_data(void *context, ble_service_data_t *srv_data)
{
    ble_manager_context_t *ctx = (ble_manager_context_t *)context;
    EXCEPTION_CHECK_INT(ctx);
    EXCEPTION_CHECK_INT(srv_data);
    if (!srv_data->p_data || srv_data->size == 0) {
        LOGE("recv data is NULL or size is 0");
        return -1;
    }
    if (srv_data->p_data[0] == '#') {
        LOG_BLE("recv_data str %s", srv_data->p_data);
    } else {
        LOG_HEX("recv_data hex", srv_data->p_data, srv_data->size);
    }
    ble_factory_shell_handle(srv_data->p_data, srv_data->size);
    ctx->dispatch_ble_client(ctx, DEVICE_BLE_FAC_MSG_CHANNEL, srv_data->p_data, srv_data->size);
    return 0;
}

int ble_factory_srv_send_data(void *context, ble_service_data_t *srv_data)
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
