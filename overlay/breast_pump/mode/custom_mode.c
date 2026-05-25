/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_mode.h"
#include "breast_pump_policy.h" // IWYU pragma: keep
#include "pump_mode_default_config.h"

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
    pump_mode_steps_info_t steps_info;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_UINT(storage_ctx);
    storage_ctx->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, &steps_info);
    /// 如果标识是默认custom策略定义，则使用自定义模式参数
    if (steps_info.id.dat32 != 0x00) {
       s_custom_obj = pump_mode_parameter_init_by_steps_info(&steps_info, "custom");
    } else {
       s_custom_obj = breast_pump_mode_parameter_init(cfg, CUSTOM_MODE_LEN, "custom");
    }
    return s_custom_obj == NULL ? -1 : 0;
}

int custom_mode_update_by_steps_info(pump_mode_steps_info_t *steps_info)
{
    motor_policy_manager_t *new_obj = NULL;
    new_obj = pump_mode_parameter_init_by_steps_info(steps_info, "custom");
    if (new_obj) {
        breast_pump_mode_parameter_deinit(s_custom_obj);
        s_custom_obj = new_obj;
        return 0;
    }
    LOGE("custom mode update by steps info failed");
    return -1;
}
 
 
 