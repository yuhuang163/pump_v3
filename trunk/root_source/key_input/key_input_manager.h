#ifndef __INPUT_MANAGER_H__
#define __INPUT_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

#ifndef CUST_KEY_EVENT_FORCE_RESET
#define CUST_KEY_EVENT_FORCE_RESET                   KEY_EVENT_LONG_10S
#endif

typedef struct {
    const char *tag;
    uint16_t keycode;
    uint16_t keytype;
    on_key_event_handler_fptr cb;
    uint8_t shield_level;   /// key_handler_shield_level_e
    uint8_t reserved[3];
} key_handler_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 







