#ifndef __APP_PROTOCOL_AUTH_SERVER_H__
#define __APP_PROTOCOL_AUTH_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

//auth
void app_old_comm_handle_get_triple_req(void);
void app_old_comm_handle_set_productid_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_deviceid_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_encryption_key_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_sn_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_get_sn_req(void);

#ifdef __cplusplus
}
#endif

#endif 
