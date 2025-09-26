#ifndef __DEVICE_REGISTER_H__
#define __DEVICE_REGISTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_list_api.h" // IWYU pragma: keep

/*
* @brief 设备注册函数声明.
*/
typedef void *(*device_register_fptr)(list_head_t *list_head);
#define DECLARE_DEVICE_REGISTER(__func_name) void * __func_name(list_head_t *list_head)

DECLARE_DEVICE_REGISTER(register_device_for_lcd);
DECLARE_DEVICE_REGISTER(register_device_for_pmu);
DECLARE_DEVICE_REGISTER(register_device_for_database);
DECLARE_DEVICE_REGISTER(register_device_for_flash);
DECLARE_DEVICE_REGISTER(register_device_for_input_key);
DECLARE_DEVICE_REGISTER(register_device_for_battery);
DECLARE_DEVICE_REGISTER(register_device_for_imu);
DECLARE_DEVICE_REGISTER(register_device_for_force_sensor);
DECLARE_DEVICE_REGISTER(register_device_for_motor);
DECLARE_DEVICE_REGISTER(register_device_for_log_uart);
DECLARE_DEVICE_REGISTER(register_device_for_led);
DECLARE_DEVICE_REGISTER(register_device_for_audio);
DECLARE_DEVICE_REGISTER(register_device_for_power);
DECLARE_DEVICE_REGISTER(register_device_for_irq);
DECLARE_DEVICE_REGISTER(register_device_for_ble);
DECLARE_DEVICE_REGISTER(register_device_for_wifi);
DECLARE_DEVICE_REGISTER(register_device_for_uart);

/*
* @brief 模块注册函数声明.
*/
typedef void *(*module_register_fptr)(list_head_t *list_head);
#define DECLARE_MODULE_REGISTER(__func_name) void * __func_name(list_head_t *list_head)

DECLARE_MODULE_REGISTER(register_module_for_peripheral_bus_new);
DECLARE_MODULE_REGISTER(register_module_for_pm_new);
DECLARE_MODULE_REGISTER(register_module_for_storage_new);
DECLARE_MODULE_REGISTER(register_module_for_res_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_time_system_new);
DECLARE_MODULE_REGISTER(register_module_for_window_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_led_festoon_policy_new);
DECLARE_MODULE_REGISTER(register_module_for_key_input_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_sensor_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_ble_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_ota_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_misc_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_breath_pump_new);
DECLARE_MODULE_REGISTER(register_module_for_algo_fusion_new);
DECLARE_MODULE_REGISTER(register_module_for_motor_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_audio_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_airplane_mode_new);
DECLARE_MODULE_REGISTER(register_module_for_wifi_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_iot_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_video_capture_manager_new);
DECLARE_MODULE_REGISTER(register_module_for_data_burial_new);

#ifdef __cplusplus
}
#endif

#endif 
