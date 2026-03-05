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

/// 追加排空模式
static motor_policy_manager_t *s_emptying_boost_obj = NULL;

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
const motor_policy_manager_t *emptying_boost_mode_policy_get(void *user_data)
{
    return s_emptying_boost_obj;
}

int emptying_boost_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_EMPTYING_BOOST_MODE);
    s_emptying_boost_obj = breast_pump_mode_parameter_init(cfg, EMPTYING_BOOST_MODE_LEN, "emptying_boost");
    // LOG_MOTOR("obj:%p", s_emptying_boost_obj);
    return s_emptying_boost_obj == NULL ? -1 : 0;
}

