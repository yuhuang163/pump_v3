#ifndef __DATA_BURY_PROTOCOL_SERVER_CLIENT_H__
#define __DATA_BURY_PROTOCOL_SERVER_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "servers_manager_api.h"

void data_bury_protocol_server_client_init(void);
int data_bury_server_data_publish(servers_publish_data_id_e id, servers_publish_data_args_t *args);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_PROTOCOL_SERVER_CLIENT_H__ */

