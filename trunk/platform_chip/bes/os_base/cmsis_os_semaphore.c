/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"

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
os_semaphore_id os_semaphore_create_wrapper (const os_semaphore_def_t *semaphore_def, int32_t count)
{ 
    (void)semaphore_def;
    return osSemaphoreNew(count, 0, NULL);
}

os_status_e os_semaphore_wait_wrapper (os_semaphore_id semaphore_id, uint32_t millisec)
{
    return osSemaphoreAcquire(semaphore_id, millisec);
}

os_status_e os_semaphore_release_wrapper (os_semaphore_id semaphore_id)
{
    return osSemaphoreRelease(semaphore_id);
}

os_status_e os_semaphore_delete_wrapper (os_semaphore_id semaphore_id)
{
    return osSemaphoreDelete(semaphore_id);
}
