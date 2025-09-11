#ifndef __TIME_SYSTEM_API_H__
#define __TIME_SYSTEM_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>
#include "broadcast_manager_api.h"

typedef struct time_system_context {
/// private
    struct __time_system_private {
        uint32_t timestamp;
        broadcast_obj_t *time_bcd;
    } priv;

/// public
    void (*sync_time)(uint32_t timestamp);
    uint32_t (*get_time)(void);
} time_system_t;


#ifdef __cplusplus
}
#endif

#endif
