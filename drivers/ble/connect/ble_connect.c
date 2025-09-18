/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"  // IWYU pragma: keep
#include "gatt_service.h" // IWYU pragma: keep
#include "ble_core_common.h" // IWYU pragma: keep
#include "ble_connect.h"
#include "app_ble.h"

/*
* DEFINES
*****************************************************************************************
*/
#define BLE_CONNECT_INTERVAL_MIN          20 // unit 1.25ms
#define BLE_CONNECT_INTERVAL_MAX          20 // unit 1.25ms
#define BLE_CONNECT_LATENCY               0x00 // max peripheral latency in units of subrated conn intervals
#define BLE_CONNECT_SUPERV_TIMEOUT_MS     (400) // unit 10ms, 100ms to 32s

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
void ble_custom_gap_connect_init(gap_conn_prefer_params_t *params)
{
    params->conn_interval_min_1_25ms = BLE_CONNECT_INTERVAL_MIN; 
    params->conn_interval_max_1_25ms = BLE_CONNECT_INTERVAL_MAX; 
    params->max_peripheral_latency = BLE_CONNECT_LATENCY;
    params->superv_timeout_ms = BLE_CONNECT_SUPERV_TIMEOUT_MS;
}

void ble_connect_params_set(gap_conn_params_wrapper_t *params)
{
    gap_conn_prefer_params_t gap_params = {0};
    gap_params.conn_interval_min_1_25ms = params->conn_interval_min; 
    gap_params.conn_interval_max_1_25ms = params->conn_interval_max; 
    gap_params.max_peripheral_latency = params->peripheral_latency;
    gap_params.superv_timeout_ms = params->superv_timeout_ms;
    gap_set_peripheral_prefer_conn_parameters(&gap_params);
}

void ble_custom_gap_disconnect(void)
{
    app_ble_disconnect_all();
}
