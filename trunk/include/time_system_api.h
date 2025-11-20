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

#define dump_human_time(u_time, timestemp) \
do { \
    static uint32_t counter = 0; \
    LOG_TIME("counter:%lu, %lu, %04d-%02d-%02d %02d:%02d:%02d week: %d\n", \
         (++counter), timestemp, (u_time)->year + 2000, (u_time)->month, (u_time)->day, \
         (u_time)->hour, (u_time)->minute, (u_time)->second, (u_time)->week); \
} while (0)

#define dump_timestamp(timestemp) \
do { \
    utc_time_t u_time; \
    timestamp_to_utc_time(timestemp, (&u_time)); \
    dump_human_time((&u_time), timestemp); \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
