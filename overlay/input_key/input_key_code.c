/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "input_key_code.h"  // IWYU pragma: keep

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
const char *get_input_key_code_name(uint16_t keycode)
{
    switch (keycode) {
        default:
            return "@unknow";
        case KEY_CODE_POWER:
            return "@power";
        case KEY_CODE_START:
            return "@start";
    #if V3_UX_VERSION == V3_UX_V3
        case KEY_CODE_STIMULATION_MODE:
            return "@stimulation";
        case KEY_CODE_LACTATION_MODE:
            return "@lactation";
        case KEY_CODE_MIX_MODE:
            return "@mix";    
    #elif (V3_UX_VERSION == V3_UX_V2 || V3_UX_VERSION == V3_UX_V4 || V3_UX_VERSION == V3_UX_V5)
        case KEY_CODE_MODE:
            return "@mode";
        case KEY_CODE_BREASTFEEDING:
            return "@breastfeeding";
        case KEY_CODE_CANCEL:
            return "@cancel";
    #elif V3_UX_VERSION == V3_UX_V6
        case KEY_CODE_STIMULATION_MODE:
            return "@stimulation";
        case KEY_CODE_LACTATION_MODE:
            return "@lactation";
        case KEY_CODE_FREQUENCY:
            return "@frequency";
        case KEY_CODE_BOTH_CONTROL:
            return "@both_control";
    #else
        case KEY_CODE_MODE:
            return "@mode";
        case KEY_CODE_BREASTFEEDING:
            return "@breastfeeding";
        case KEY_CODE_FREQUENCY:
            return "@frequency";
    #endif
        case KEY_CODE_FACTORY_RESET:
            return "@factory_reset";
        case KEY_CODE_TRAVEL_LOCK:
            return "@travel_lock";
        case KEY_CODE_LEFT_CONTROL:
            return "@left_control";
        case KEY_CODE_RIGHT_CONTROL:
            return "@right_control";
        case KEY_CODE_KNOB_LEFT:
            return "@knob_left";
        case KEY_CODE_KNOB_RIGHT:
            return "@knob_right";
    }
}