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

/// 2+8奶量提升模式
static motor_policy_manager_t *s_milk_boost_obj = NULL;

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
const motor_policy_manager_t *milk_boost_mode_policy_get(void *user_data)
{
    return s_milk_boost_obj;
}

int milk_boost_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_MILK_BOOST_MODE);
    s_milk_boost_obj = breast_pump_mode_parameter_init(cfg, MILK_BOOST_MODE_LEN, "milk_boost");
    // LOG_MOTOR("obj:%p", s_milk_boost_obj);
    return s_milk_boost_obj == NULL ? -1 : 0;
}



 
 
 