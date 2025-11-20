#ifndef __BREAST_PUMP_MANAGER_H__
#define __BREAST_PUMP_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

#define PUMP_WORK_STEP_TIME_MS   (100)
#define BREAST_PUMP_IS_GOING(state) (state == BREAST_PUMP_START || state == BREAST_PUMP_CONTINUE)
#define BREAST_PUMP_IS_COMMON_MODE(mode) (mode >= BREAST_PUMP_STIMULATION_MODE && mode <= BREAST_PUMP_MIX_MODE)

#define PUMP_LOCK_LOG(...)
#define PUMP_THD_WAKELOCK                     "pump_wakelock"

#define MUTEX_LOCK_BHT(__tag, __lock) \
do { \
    PUMP_LOCK_LOG("Take lock=>__tag: %s 00", __tag); \
    MUTEX_LOCK(__lock); \
} while (0)

#define MUTEX_UNLOCK_BHT(__tag, __lock) \
do { \
    MUTEX_UNLOCK(__lock); \
    PUMP_LOCK_LOG("Give lock=>__tag: %s", __tag); \
} while (0)

typedef struct {
    void *user_data;
    on_breast_pump_result_cb_ptr cb;
    list_head_t list;
} breast_pump_evt_client_node_t;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 







