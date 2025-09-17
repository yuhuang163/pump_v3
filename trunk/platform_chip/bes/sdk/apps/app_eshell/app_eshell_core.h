#ifndef __APP_ESHELL_CORE_H__
#define __APP_ESHELL_CORE_H__
#include "plat_types.h"
#include "app_rpc_api.h"

#if defined(APP_RPC_BTH_M55_EN)
#include "app_dsp_m55.h"
#define ESHELL_TASK_CMD_TO_ADD          CORE_BRIDGE_TASK_COMMAND_TO_ADD
#define APP_RPC_CORE_REMOTE             (APP_RPC_CORE_BTH_M55)
#endif

#if defined(APP_RPC_BTH_DSP_EN)
#include "rpc_bth_dsp.h"
#define ESHELL_TASK_CMD_TO_ADD          RPC_BTH_DSP_TASK_CMD_TO_ADD
#define APP_RPC_CORE_REMOTE             (APP_RPC_CORE_BTH_DSP)
#endif

#if defined(APP_RPC_MCU_SENSOR_EN)
#include "app_sensor_hub.h"
#define ESHELL_TASK_CMD_TO_ADD          CORE_BRIDGE_TASK_COMMAND_TO_ADD
#define APP_RPC_CORE_REMOTE             (APP_RPC_CORE_MCU_SENSOR)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(APP_RPC_ENABLE)
typedef void (*eshell_rpc_send_done_callback_t)(void);
typedef void (*eshell_rpc_recv_process_callback_t)(unsigned char *ptr, short len);

void app_esehll_rpc_bridge_cmd_send_done_handler_cb_register(eshell_rpc_send_done_callback_t handler);
void app_esehll_rpc_bridge_cmd_recv_handler_cb_register(eshell_rpc_recv_process_callback_t handler);
#endif  //end of APP_RPC_ENABLE

#ifdef __cplusplus
}
#endif
#endif //end of __APP_ESHELL_CORE_H__