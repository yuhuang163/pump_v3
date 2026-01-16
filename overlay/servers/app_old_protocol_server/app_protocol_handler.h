#ifndef __APP_PROTOCOL_HANDLER_H__
#define __APP_PROTOCOL_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "app_old_protocol_parser.h"
#include "app_protocol_pump_ctrl_server.h"
#include "app_protocol_sys_ctrl_server.h"
#include "app_protocol_auth_server.h"
#include "app_protocol_taste_ctrl_server.h"

typedef struct app_old_protocol_handler_context {
    protocol_manager_context_t *protocol_ctx;
    uint8_t encrypt_status;
    uint8_t encrypt_key;
    app_old_comm_pump_taste_ctrl taste_ctrl;
} app_old_protocol_handler_context_t;

void app_old_comm_handle_request_cmd(app_old_comm_cmd_id_e cid, uint8_t cal, uint8_t *cab);
void app_old_comm_handle_response_cmd(app_old_comm_cmd_id_e ct, uint8_t cal, void *cab);
void app_old_protocol_data_cryption(uint8_t key, uint8_t *dat, uint8_t len);
void rc4(unsigned char *key, unsigned int keyLength, unsigned char *data, unsigned int dataLength);
void app_old_protocol_ctx_init(void);
void app_old_protocol_hander_frame_builder_send(app_old_commcmd_type_e cmdtype, app_old_comm_cmd_id_e cid,
                                                uint8_t data_len, uint8_t *data);
app_old_protocol_handler_context_t *app_old_protocol_handler_ctx_get(void);
#ifdef __cplusplus
}
#endif

#endif 