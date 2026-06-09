#ifndef __COMMON_SERVICE_BASE_H__
#define __COMMON_SERVICE_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "stdint.h" // IWYU pragma: keep
#include "comm_protocol_defs.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
int algo_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
int pump_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
int iot_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
int sensors_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
int sys_config_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
int sys_state_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
int tests_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
int tracking_data_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
int ota_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);

#ifdef __cplusplus
}
#endif

#endif // __PROTOCOL_SERVICE_H__

