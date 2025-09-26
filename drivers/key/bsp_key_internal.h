#ifndef __BSP_KEY_INTERNAL_H__
#define __BSP_KEY_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdio.h>
#include <stdint.h>


/**@brief Firmware information define*/


/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int bsp_key_init(void);
int bsp_key_deinit(void);
int bsp_read_key_value(uint8_t key_index);
int bsp_key_sensitivity_set(uint8_t key_index, uint8_t sensitivity);

#ifdef __cplusplus
}
#endif

#endif 
