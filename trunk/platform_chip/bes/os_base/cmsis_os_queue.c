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
os_status_e os_message_put_wrapper (os_message_id queue_id, void *msg, uint32_t millisec)
{
    return osMessageQueuePut(queue_id, msg, 0, millisec);
}

os_status_e os_message_get_wrapper (os_message_id queue_id, void *msg, uint32_t millisec)
{
    return osMessageQueueGet(queue_id, msg, NULL, millisec);
}
 
os_status_e os_message_delete_wrapper (os_message_id queue_id)
{
    os_status_e status = osMessageQueueDelete(queue_id);
    if (status != osOK) {
        LOGE("Oops! osMessageQueueDelete: fail %p", queue_id);
        return status;
    }
    return status;
}
 
os_status_e os_message_reset_wrapper (os_message_id queue_id)
{
    return osMessageQueueReset(queue_id);
}

int os_create_msg_queue_wrapper(os_message_id *msg_ptr, int queue_num, int item_size)
{
    *msg_ptr = osMessageQueueNew(queue_num, item_size, NULL);
    if (*msg_ptr == NULL) {
        LOGE("Oops! queue create failed");
        return -1;
    } 
    return 0;
}
