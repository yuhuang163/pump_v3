#ifndef __MASTER_SERVER_CLIENT_H__
#define __MASTER_SERVER_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "stdint.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void master_server_client_init(void);
int on_master_app_old_protocol_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
int app_old_protocol_server_data_publish(app_old_comm_cmd_id_e cid, servers_publish_data_args_t *args);

#ifdef __cplusplus
}
#endif

#endif // __PROTOCOL_SERVICE_H__

