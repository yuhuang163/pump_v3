#ifndef __BLE_EVENT_PROCESS_H__
#define __BLE_EVENT_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ble_message_process_handler(ble_manager_context_t *ctx, ble_event_type_t *msg);
#ifdef __cplusplus
}
#endif

#endif 







