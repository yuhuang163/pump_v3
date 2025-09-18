#ifndef __BLE_ADVS_H__
#define __BLE_ADVS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h"

typedef struct {
    uint16_t conn_interval_min; // unit 1.25ms, 7.5ms to 4000ms
    uint16_t conn_interval_max; // unit 1.25ms, 7.5ms to 4000ms
    uint16_t peripheral_latency; // max peripheral latency in units of subrated conn intervals
    uint16_t superv_timeout_ms; // unit 10ms, 100ms to 32s
} ble_advs_params_wrapper_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ble_custom_gap_advs_start(void);
void ble_custom_gap_advs_stop(void);

#ifdef __cplusplus
}
#endif

#endif
