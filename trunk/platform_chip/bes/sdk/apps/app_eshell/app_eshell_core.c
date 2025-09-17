#include <string.h>
#include "plat_types.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "app_rpc_api.h"
#include "app_eshell_core.h"

#if defined(APP_RPC_ENABLE)
static eshell_rpc_send_done_callback_t g_eshell_corebridge_cmd_tx_done_cb_handler = NULL;
static eshell_rpc_recv_process_callback_t g_eshell_corebridge_cmd_recv_done_cb_handler = NULL;

static void app_esehll_bridge_send_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_rpc_send_data_no_rsp(APP_RPC_CORE_REMOTE, MCU_BTH_M55_TASK_CMD_ESHELL_TRANSMIT_DATA, ptr, len);
}

static void app_esehll_bridge_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    if (g_eshell_corebridge_cmd_tx_done_cb_handler) {
        g_eshell_corebridge_cmd_tx_done_cb_handler();
    }
}

static void app_esehll_bridge_recv_cmd_handler(uint8_t* ptr, uint16_t len)
{
    if (g_eshell_corebridge_cmd_recv_done_cb_handler) {
        g_eshell_corebridge_cmd_recv_done_cb_handler(ptr, len);
    }
}

void app_esehll_rpc_bridge_cmd_send_done_handler_cb_register(eshell_rpc_send_done_callback_t handler)
{
    g_eshell_corebridge_cmd_tx_done_cb_handler = handler;
}

void app_esehll_rpc_bridge_cmd_recv_handler_cb_register(eshell_rpc_recv_process_callback_t handler)
{
    g_eshell_corebridge_cmd_recv_done_cb_handler = handler;
}

ESHELL_TASK_CMD_TO_ADD(MCU_BTH_M55_TASK_CMD_ESHELL_TRANSMIT_DATA,
                            "eshell_corebridge_comm",
                            app_esehll_bridge_send_cmd_handler,
                            app_esehll_bridge_recv_cmd_handler,
                            0,
                            NULL,
                            NULL,
                            app_esehll_bridge_tx_done_handler);
#endif