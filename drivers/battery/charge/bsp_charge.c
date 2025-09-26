/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_battery_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define  CHARGING_STATUS_VALUE               0x02
#define  CHARHE_FULL_STATUS_VALUE            0x01
#define  UNCHARGE_STATUS_VALUE               0x00 

#define  CHARHE_CHECK_NO_BATT_0              0xFF 
#define  CHARHE_CHECK_NO_BATT_1              0xFE 
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const char *charge_status_arr[] = {
    "charge error",
    "charge full",
    "charging", 
    "uncharge",
    "no battery",
};

// static uint16_t charge_full_debounce = 0;
// static void (*s_charge_irq_callback)(void *param);

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int get_charge_level(uint8_t index)
{
    return 0;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
bool vbus_is_plug_in(void)
{
    return false;
}

int bsp_charge_gpio_init(void)
{
    return 0;
}

void add_charge_irq_callback(void (*cb)(void *param))
{
    // s_charge_irq_callback = cb;
}

int get_charge_state(void) 
{
    return 0;
}

int get_charge_full_debounce(void) 
{
    return 0;
}

void charge_irq_event_callback(void)
{

}
