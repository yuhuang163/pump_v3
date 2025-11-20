#ifndef __SENSOR_MANAGER_H__
#define __SENSOR_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

#define SENSOR_THREAD_POLLING_PERIOD         (10)

typedef struct {
    void *user_data;
    on_sensor_evt_data_cb_fptr cb;
    int report_type;
    int step;
    uint32_t old_cnt;
    list_head_t list;
} sensor_evt_client_node_t;

#ifdef __cplusplus
}
#endif

#endif 







