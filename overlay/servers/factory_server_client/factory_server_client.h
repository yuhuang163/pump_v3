#ifndef __FACTORY_SERVER_CLIENT_H__
#define __FACTORY_SERVER_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "stdint.h" // IWYU pragma: keep
#include "servers_manager_api.h" // IWYU pragma: keep
/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void factory_server_client_init(void);
int factory_server_data_publish(servers_publish_data_id_e id, servers_publish_data_args_t *args);
#ifdef __cplusplus
}
#endif

#endif // __PROTOCOL_SERVICE_H__

