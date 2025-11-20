#ifndef __BLE_FACTORY_SHELL_H__
#define __BLE_FACTORY_SHELL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ble_manager_api.h" // IWYU pragma: keep

/// ble shell 命令函数类型
typedef void (*ble_shell_func)(const uint8_t *param, uint16_t len);
typedef struct _ble_shell_cmd {
    char *cmd_str;
    ble_shell_func cmd_fun;
} ble_factory_shell_t;

void ble_factory_shell_handle(uint8_t *p_data, uint16_t len);
#ifdef __cplusplus
}
#endif

#endif 


