#ifndef __HANG_DETECT_H__
#define __HANG_DETECT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#ifndef IS_SUPPORT_HANG_DETECT
#define IS_SUPPORT_HANG_DETECT                             (0)
#endif

#if IS_SUPPORT_HANG_DETECT

void add_to_hang_detect_list(os_mutex_id lock, const char *lock_name, int max_hold_time_s);
void remove_from_hang_detect_list(os_mutex_id lock);
void init_hang_detect_context(void);
os_status_e hang_detect_mutex_lock(os_mutex_id mutex_id, uint32_t timeout);
os_status_e hang_detect_mutex_unlock(os_mutex_id mutex_id);

#undef MUTEX_LOCK_HG
#undef MUTEX_UNLOCK_HG
#undef MUTEX_INIT
#undef MUTEX_DELETE
#define MUTEX_LOCK_HG(x, timeout)       hang_detect_mutex_lock((x), timeout)
#define MUTEX_UNLOCK_HG(x)              hang_detect_mutex_unlock((x))
#define MUTEX_INIT_HOLD(x, name, max_hold_time_s) \
    do { \
        MUTEX_INIT_HG(x, name); \
        add_to_hang_detect_list(*(x), TO_STR(name), (max_hold_time_s)); \
    } while (0)
#define MUTEX_INIT(x, name)             MUTEX_INIT_HOLD(x, name, 0)
#define MUTEX_LOCK(x)                   MUTEX_LOCK_HG(x, OS_WAIT_FOREVER)
#define MUTEX_UNLOCK(x)                 MUTEX_UNLOCK_HG(x)
#define MUTEX_DELETE(x) \
    do { \
        remove_from_hang_detect_list(x); \
        MUTEX_DELETE_HG(x); \
    } while (0)

#define HANG_DETECT_LOCK_INIT(lock_ptr, name, sec) \
    MUTEX_INIT_HOLD(lock_ptr, name, sec)
#define HANG_DETECT_LOCK(lock)          MUTEX_LOCK(lock)
#define HANG_DETECT_UNLOCK(lock)        MUTEX_UNLOCK(lock)

#else

#define add_to_hang_detect_list(lock, name, sec)           ((void)0)
#define remove_from_hang_detect_list(lock)                 ((void)0)
#define init_hang_detect_context()                         ((void)0)

#define HANG_DETECT_LOCK_INIT(lock_ptr, name, sec)         ((void)0)
#define HANG_DETECT_LOCK(lock)                             ((void)0)
#define HANG_DETECT_UNLOCK(lock)                           ((void)0)

#endif

#ifdef __cplusplus
}
#endif

#endif
