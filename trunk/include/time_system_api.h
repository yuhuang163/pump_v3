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

typedef void (*on_time_elapsed_cb_fptr)(void *user_data);

typedef struct {
    int (*init)(void);
    on_time_elapsed_cb_fptr callback;
} rtc_bsp_interface_t;

typedef struct time_system_context {
// private
    uint32_t timestamp;
    rtc_bsp_interface_t rtc_if;
    broadcast_obj_t receiver;

// public
    void (*sync_time)(uint32_t timestamp);
    uint32_t (*get_time)(void);
} time_system_t;

/* PUBLIC FUNCTION ---------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif
