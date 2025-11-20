#ifndef __BROADCAST_DATA_DEFINE_H__
#define __BROADCAST_DATA_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "root_list_api.h" // IWYU pragma: keep
#include "handler_post.h" // IWYU pragma: keep

typedef enum {
    ID_BROADCAST_SYS_POWER_OFF = 100,
    ID_BROADCAST_SYS_POWER_ON,
} broadcast_sys_power_state_e;

#ifdef __cplusplus
}
#endif

#endif 


