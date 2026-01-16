#ifndef __BLE_MANAGER_H__
#define __BLE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

typedef struct {
    void *user_data;
    int msg_channel; /// 消息通道
    on_ble_evt_data_cb_fptr cb;
    list_head_t list;
} ble_evt_client_node_t;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 







