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

/// 放松模式
static motor_policy_manager_t *s_relax_obj = NULL;

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
const motor_policy_manager_t *relax_mode_policy_get(void *user_data)
{
    return s_relax_obj;
}

int relax_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_RELAX_MODE);
    s_relax_obj = breast_pump_mode_parameter_init(cfg, RELAX_MODE_LEN, "relax");
    // LOG_MOTOR("obj:%p", s_relax_obj);
    return s_relax_obj == NULL ? -1 : 0;
}

