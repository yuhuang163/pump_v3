#ifndef __FACTORY_PROTOCOL_PARSE_H__
#define __FACTORY_PROTOCOL_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

void on_perpare_factory_protocol_init(protocol_manager_context_t *ctx);
int on_factory_protocol_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
void on_post_factory_protocol_init(protocol_manager_context_t *ctx);
uint8_t product_tests_protocol_parse_route(uint8_t *data, uint16_t len);
#ifdef __cplusplus
}
#endif

#endif 