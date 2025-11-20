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

/// 刺激模式
static motor_policy_manager_t *s_stimulation_obj = NULL;

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
const motor_policy_manager_t *stimulation_mode_policy_get(void *user_data)
{
    return s_stimulation_obj;
}

int stimulation_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_STIMULATION_MODE);
    s_stimulation_obj = breast_pump_mode_parameter_init(cfg, STIMULATION_MODE_LEN, "stimulation");
    // LOG_MOTOR("obj:%p", s_stimulation_obj);
    return s_stimulation_obj == NULL ? -1 : 0;
}

