#ifndef __BSP_BLE_INTERNAL_H__
#define __BSP_BLE_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h"  // IWYU pragma: keep
#include "master_service.h"  // IWYU pragma: keep
#include "factory_service.h" // IWYU pragma: keep
#include "remot_service.h" // IWYU pragma: keep
#include "data_bury_service.h" // IWYU pragma: keep
#include "ble_service.h" // IWYU pragma: keep
#include "ble_advs.h" // IWYU pragma: keep
#include "ble_connect.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
******************************************************************************
*/
uint16_t ble_custom_mtu_size_get(void);
void ble_custom_tx_ccc_changed(uint8_t conidx, uint16_t connhdl, bool notify_enabled, uint8_t service_id);
#ifdef __cplusplus
}
#endif

#endif 
