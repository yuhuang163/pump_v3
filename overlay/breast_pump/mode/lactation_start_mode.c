/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_mode.h"
#include "breast_pump_policy.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/// 开奶模式
static motor_policy_manager_t *s_lactation_start_obj = NULL;

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
const motor_policy_manager_t *lactation_start_mode_policy_get(void *user_data)
{
    return s_lactation_start_obj;
}

int lactation_start_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_LACTATION_START_MODE);
    s_lactation_start_obj = breast_pump_mode_parameter_init(cfg, LACTATION_START_MODE_LEN, "lactation_start");
    // LOG_MOTOR("obj:%p", s_lactation_start_obj);
    return s_lactation_start_obj == NULL ? -1 : 0;
}



 
 
 