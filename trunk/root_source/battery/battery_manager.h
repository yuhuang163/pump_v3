#ifndef __BATTERY_MANAGER_H__
#define __BATTERY_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

typedef struct {
    void *user_data;
    on_battery_state_cb_ptr cb;
    list_head_t list;
} battery_evt_client_node_t;

#ifdef __cplusplus
}
#endif

#endif 


