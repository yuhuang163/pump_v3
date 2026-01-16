/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "framework_hook.h" // IWYU pragma: keep
#include "root_common_api.h"

/*
 * DEFINES
 *****************************************************************************************
 */
#define POPUP_WM_TIME_BY_APP                ( 1500 )  // APP 点击 弹窗时间

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

// hook点入口.
void framework_extra_hook(void)
{
    /// 注册存储钩子.
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    CHECK_IF_VALID(storage_ctx, {
        register_storage_hook(storage_ctx);
    });
    /// 注册输入按键钩子.
    key_input_context_t *input_key_ctx = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    CHECK_IF_VALID(input_key_ctx, {
        register_input_key_hook(input_key_ctx);
    });
    /// 注册电机钩子.
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    CHECK_IF_VALID(motor_ctx, {
        register_motor_hook(motor_ctx);
    });
    /// 注册传感器钩子.
    sensor_manager_context_t *sensor_ctx = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    CHECK_IF_VALID(sensor_ctx, {
        register_sensor_hook(sensor_ctx);
    });
    /// 注册泵奶钩子.
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    CHECK_IF_VALID(pump_ctx, {
        register_breast_pump_hook(pump_ctx);
    });
    /// 注册电源管理钩子.
    power_manager_context_t *power_ctx = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    CHECK_IF_VALID(power_ctx, {
        register_power_manager_hook(power_ctx);
    });
    /// 注册飞行模式钩子.
    airplane_mode_context_t *airplane_ctx = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    CHECK_IF_VALID(airplane_ctx, {
        register_airplane_mode_hook(airplane_ctx);
    });
    /// 注册UI管理钩子.
    ui_manager_context_t *ui_ctx = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    CHECK_IF_VALID(ui_ctx, {
        register_ui_manager_hook(ui_ctx);
    });
    /// 注册算法钩子.
    algo_manager_context_t *algo_ctx = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    CHECK_IF_VALID(algo_ctx, {
        register_algo_hook(algo_ctx);
    });
    /// 注册产品测试钩子.
    tests_manager_context_t *product_tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    CHECK_IF_VALID(product_tests_ctx, {
        register_product_tests_hook(product_tests_ctx);
    });
    /// 注册服务器管理钩子.
    servers_manager_context_t *servers_manager_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    CHECK_IF_VALID(servers_manager_ctx, {
        register_servers_manager_hook(servers_manager_ctx);
    });
}
