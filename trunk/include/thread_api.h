#ifndef __THREAD_API_H__
#define __THREAD_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>
#include "cmsis_os_api.h" // IWYU pragma: keep

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
void set_thread_running(thread_context_t *thd, bool running);
uint32_t get_thread_running(thread_context_t *thd);

#ifdef __cplusplus
}
#endif

#endif 


