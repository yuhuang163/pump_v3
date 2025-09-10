/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void set_thread_running(thread_context_t *thd, bool running)
{
    if (!thd || !thd->lock_id) {
        LOGE("err");
        return;
    }

    MUTEX_LOCK(thd->lock_id);
    if (running) {
        thd->is_running = get_timestamp();
    } else {
        thd->is_running = 0;
    }
    MUTEX_UNLOCK(thd->lock_id);
}

uint32_t get_thread_running(thread_context_t *thd)
{
    uint32_t rc;

    if (!thd || !thd->lock_id) {
        LOGE("err");
        return 0;
    }

    MUTEX_LOCK(thd->lock_id);
    rc = thd->is_running;
    MUTEX_UNLOCK(thd->lock_id);

    return rc;
}

