#ifndef __OTA_MANAGER_H__
#define __OTA_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdio.h> // IWYU pragma: keep
#include <stdlib.h> // IWYU pragma: keep

#define DISPATCH_OTA_EVENT(context, _event, _sub_code) \
    dispatch_ota_event_wrapper(context, _event, _sub_code)

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int dispatch_ota_event_wrapper(void *context, int event, uint32_t sub_code);

#ifdef __cplusplus
}
#endif

#endif 


