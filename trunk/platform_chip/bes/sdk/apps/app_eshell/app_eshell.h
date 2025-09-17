#ifndef __APP_ESHELL_H__
#define __APP_ESHELL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EXT_ESHELL_MSG_SUB_CMD_OP_CMD = 0,
    EXT_ESHELL_MSG_SUB_CMD_OP_RESULT,
    EXT_ESHELL_MSG_SUB_CMD_MAX_NUM,
} EXT_ESHELL_MSG_SUB_CMD_CODE_E;

typedef enum {
    ESHELL_CMD_DIR_TRANSIMIT,
    ESHELL_CMD_DIR_RSP,
} EXT_ESHELL_MSG_CMD_DIR_E;

#if defined(APP_RPC_ENABLE)
void app_eshell_rpc_send_op_cmd_func(char *buffer, unsigned int buff_size);
void app_eshell_rpc_send_op_result_func(char *buffer, unsigned int buff_size);
void app_eshell_rpc_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // end of __APP_ESHELL_H__
