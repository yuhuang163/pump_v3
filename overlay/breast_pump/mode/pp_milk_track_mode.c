/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_mode.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/// pp追奶模式
static motor_policy_manager_t *s_pp_milk_track_obj = NULL;

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
const motor_policy_manager_t *pp_milk_track_mode_policy_get(void *user_data)
{
    return s_pp_milk_track_obj;
}

int pp_milk_track_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_PP_MILK_TRACK_MODE);
    s_pp_milk_track_obj = breast_pump_mode_parameter_init(cfg, PP_MODE_LEN, "pp_milk_track");
    // LOG_MOTOR("obj:%p", s_pp_milk_track_obj);
    return s_pp_milk_track_obj == NULL ? -1 : 0;
}
