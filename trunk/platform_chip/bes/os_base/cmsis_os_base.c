/*
* INCLUDE FILES
*****************************************************************************************
*/
#include <stdint.h>
#include <stdbool.h>
#include "rtx_lib.h"  // IWYU pragma: keep
#include "cmsis_os_api.h"  // IWYU pragma: keep

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
bool is_interrupt_handler_wrapper(void)
{
    if (IsIrqMode() || IsIrqMasked()) {
        return 1;
    } else {
        return 0;
    }
}

void msleep(uint32_t time_ms)
{
    osDelay(time_ms);
}

uint32_t get_timestamp(void)
{
    return osKernelGetSysTimerCount();
}
