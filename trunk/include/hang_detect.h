#ifndef __HANG_DETECT_H__
#define __HANG_DETECT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

void add_to_hang_detect_list(os_mutex_id lock, const char *lock_name, int max_hold_time_s);
void init_hang_detect_context(void);

#ifdef __cplusplus
}
#endif

#endif 
