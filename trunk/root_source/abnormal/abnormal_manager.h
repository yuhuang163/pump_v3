#ifndef __ABNORMAL_MANAGER_H__
#define __ABNORMAL_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_api.h" // IWYU pragma: keep

typedef struct {
    void *user_data;
    on_abnormal_state_cb_ptr cb;
    list_head_t list;
} abnormal_evt_client_node_t;

#ifdef __cplusplus
}
#endif

#endif
