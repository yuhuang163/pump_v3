/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_policy.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

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
motor_policy_manager_t *pump_total_policy_init(motor_policy_cfg_t cfg) 
{
    motor_policy_manager_t *policy_obj = u_malloz(sizeof(motor_policy_manager_t));
    if (policy_obj == NULL) {
        LOGE("policy malloc fail !!!");
        return NULL;
    }
    motor_policy_core_t *m_para = &policy_obj->head.policy.core;

    policy_obj->timed_policy_count = 1;
    policy_obj->head.sub_policy_count = 1;
    policy_obj->head.duration_seconds = cfg.duration;
    memcpy(m_para, &cfg.core, sizeof(motor_policy_core_t));
    return policy_obj;
}

motor_policy_manager_t *pump_total_policy_init_by_steps_info(pump_mode_step_param_t cfg) 
{
    motor_policy_manager_t *policy_obj = u_malloz(sizeof(motor_policy_manager_t));
    if (policy_obj == NULL) {
        LOGE("policy malloc fail !!!");
        return NULL;
    }
    motor_policy_core_t *m_para = &policy_obj->head.policy.core;
    policy_obj->timed_policy_count = 1;
    policy_obj->head.sub_policy_count = 1;
    policy_obj->head.duration_seconds = cfg.duration;
    m_para->type = MOTOR_PARAM_TABLE;
    m_para->un.mapping.speed = cfg.freq;
    m_para->un.mapping.level = cfg.level;
    m_para->un.mapping.mode_type = cfg.mode_type;
    m_para->config.loop_num = cfg.loop_num;

    return policy_obj;
}

int pump_total_policy_deint(motor_policy_manager_t *total_policy) 
{
    EXCEPTION_CHECK_INT(total_policy);
    if (total_policy->timed_policy_count > 1) {
        LOGE("total policy memory leak !!!!!!!!");
    }
    u_free(total_policy);
    total_policy = NULL;

    return 0;
}