#ifndef __OTA_SERVER_CLIENT_H__
#define __OTA_SERVER_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "servers_manager_api.h" // IWYU pragma: keep
/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ota_server_client_init(void);
int ota_server_data_publish(servers_publish_data_id_e id, servers_publish_data_args_t *args);
#ifdef __cplusplus
}
#endif

#endif // __PROTOCOL_SERVICE_H__

