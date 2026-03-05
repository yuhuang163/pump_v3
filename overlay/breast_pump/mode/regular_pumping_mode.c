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

/// 常规韵律模式
static motor_policy_manager_t *s_regular_pumping_obj = NULL;

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
const motor_policy_manager_t *regular_pumping_mode_policy_get(void *user_data)
{
    return s_regular_pumping_obj;
}

int regular_pumping_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_REGULAR_PUMPING_MODE);
    s_regular_pumping_obj = breast_pump_mode_parameter_init(cfg, REGULAR_PUMPING_MODE_LEN, "regular_pumping");
    // LOG_MOTOR("obj:%p", s_regular_pumping_obj);
    return s_regular_pumping_obj == NULL ? -1 : 0;
}

