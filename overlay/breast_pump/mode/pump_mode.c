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

/// 吸乳模式
static motor_policy_manager_t *s_pump_obj = NULL;

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
const motor_policy_manager_t *pump_mode_policy_get(void *user_data)
{
    return s_pump_obj;
}

int pump_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_PUMP_MODE);
    s_pump_obj = breast_pump_mode_parameter_init(cfg, PUMP_MODE_LEN, "pump");
    // LOG_MOTOR("obj:%p", s_pump_obj);
    return s_pump_obj == NULL ? -1 : 0;
}


