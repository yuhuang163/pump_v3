#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_key_simulation.h"
#include "input_key_code.h"
#include "key_input_manager_api.h"
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
#include "root_common_api.h" // IWYU pragma: keep

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
void app_start_key_simulation(void)
{
    key_input_context_t *key_input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_VOID(key_input);
    /// 启动按键模拟
    key_input->dispatch_client_input_key(KEY_CODE_START, KEY_EVENT_CLICK);
}

void app_mode_key_simulation(void)
{   
    key_input_context_t *key_input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_VOID(key_input);
    /// mode键按键模拟
    key_input->dispatch_client_input_key(KEY_CODE_MODE, KEY_EVENT_CLICK);
}

void app_frequency_key_simulation(void)
{
    key_input_context_t *key_input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_VOID(key_input);
    /// 频率按键模拟
}

void app_breastfeeding_key_simulation(void)
{
    key_input_context_t *key_input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_VOID(key_input);
    /// 追奶按键模拟
}

void app_left_control_key_simulation(void)
{
    key_input_context_t *key_input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_VOID(key_input);
    /// L按键模拟
    key_input->dispatch_client_input_key(KEY_CODE_LEFT_CONTROL, KEY_EVENT_CLICK);
}

void app_right_control_key_simulation(void)
{
    key_input_context_t *key_input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_VOID(key_input);
    /// R按键模拟
    key_input->dispatch_client_input_key(KEY_CODE_RIGHT_CONTROL, KEY_EVENT_CLICK);
}

void app_encoder_key_simulation(void)
{
    key_input_context_t *key_input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_VOID(key_input);
    /// 编码器按键模拟
    key_input->dispatch_client_input_key(KEY_CODE_KNOB_LEFT, KEY_EVENT_CLICK);
}