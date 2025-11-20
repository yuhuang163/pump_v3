/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_mode.h"
#include "breast_pump_policy.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/// 个性化模式
static motor_policy_manager_t *s_custom_obj = NULL;

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
const motor_policy_manager_t *custom_mode_policy_get(void *user_data)
{
    return s_custom_obj;
}

int custom_mode_parameter_init(void)
{
    const motor_policy_cfg_t *cfg = get_breast_pump_mode_para(BREAST_PUMP_CUSTOM_MODE);
    s_custom_obj = breast_pump_mode_parameter_init(cfg, CUSTOM_MODE_LEN, "custom");
    // LOG_MOTOR("obj:%p", s_custom_obj);
    return s_custom_obj == NULL ? -1 : 0;
}

int custom_mode_parameter_update(motor_policy_cfg_t *cfg, uint8_t cfg_num)
{
    motor_policy_manager_t *new_obj = NULL;
    new_obj = breast_pump_mode_parameter_init(cfg, cfg_num, "new_custom");
    if (new_obj) {
        breast_pump_mode_parameter_deinit(s_custom_obj);
        s_custom_obj = new_obj;
        // LOG_MOTOR("obj:%p", s_custom_obj);
        return 0;
    }
    // LOG_MOTOR("obj:%p", s_custom_obj);
    return -1;
}

 
 
 