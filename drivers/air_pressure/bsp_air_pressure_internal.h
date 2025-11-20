#ifndef __BSP_AIR_PRESSURE_INTERNAL_H__
#define __BSP_AIR_PRESSURE_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h" // IWYU pragma: keep

/**@brief Firmware information define*/

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int bsp_air_pressure_init(void);
int bsp_air_pressure_deinit(void);
int bsp_read_air_pressure_status(void *args);

#ifdef __cplusplus
}
#endif

#endif 
