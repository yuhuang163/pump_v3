/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_battery_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define ADC_CONV_LENGTH                 (8UL)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
// static const battery_adc_t sg_batt_adc = {
//     4.0,
//     2000,
//     4600,
// };

// static uint8_t adc_init_flag = 0;
// static os_message_id adc_msg_id = NULL;

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
int battery_adc_init(void)
{
    return 0;
}

int battery_adc_resume(void)
{
    return 0;
}

int battery_adc_suspend(void)
{
    return 0;
}

int battery_adc_single_conversion(uint16_t *batt_voltage)
{
    return 0;
}

