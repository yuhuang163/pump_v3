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

/// 混合模式
static motor_policy_manager_t *s_mix_obj = NULL;

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
const motor_policy_manager_t *mix_mode_policy_get(void *user_data)
{
    return s_mix_obj;
}

int mix_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_MIX_MODE);
    s_mix_obj = breast_pump_mode_parameter_init(cfg, MIX_MODE_LEN, "mix");
    // LOG_MOTOR("obj:%p", s_mix_obj);
    return s_mix_obj == NULL ? -1 : 0;
}


