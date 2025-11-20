#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h" // IWYU pragma: keep
#include "hal_gpadc.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int bsp_adc_single_conversion(uint8_t channel_id, uint16_t conv_times, uint16_t *voltage);
void bsp_adc_init(void);
void bsp_adc_deinit(uint8_t channel_id);

#ifdef __cplusplus
}
#endif

#endif 
