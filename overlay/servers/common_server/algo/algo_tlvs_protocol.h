#ifndef __ALGO_TLVS_PROTOCOL_H__
#define __ALGO_TLVS_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "comm_protocol_builder.h" // IWYU pragma: keep
#include "servers_manager_api.h" // IWYU pragma: keep

/// 算法结果上报缓存帧数（缓存满该帧数后统一 notify 上报）
#define ALGO_RESULT_REPORT_CACHE_NUM  5

/// TLV ID
typedef enum {
    ALGO_TLV_SAMPLE_ONOFF_SET,        /// 采测开关(0:关, 1:开)
    ALGO_TLV_SAMPLE_INTERVAL_SET,     /// 采测间隔(ms)
    ALGO_TLV_SAMPLE_ONOFF_GET,        /// 采测开关(0:关, 1:开)
    ALGO_TLV_SAMPLE_INTERVAL_GET,     /// 采测间隔(ms)
    ALGO_TLV_SAMPLE_DATA_NUM,         /// 采测数据个数
    ALGO_TLV_SAMPLE_AIR_PRESSURE_DATA,   /// 气压数据
    ALGO_TLV_SAMPLE_LIGHT_SENSOR_DATA,  ///光感数据
    ALGO_TLV_RESULT_REPORT_ONOFF_SET = 7, /// 算法结果上报开关(0:关, 1:开)
    ALGO_TLV_RESULT_REPORT = 8,           /// 算法结果上报(notify)

} algo_tlvs_id_e;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
int algo_tlvs_sample_onoff_set(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int algo_tlvs_sample_interval_set(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int algo_tlvs_sample_onoff_get(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int algo_tlvs_sample_interval_get(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int algo_tlvs_result_report_onoff_set(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int algo_sampling_data_publish(servers_publish_data_args_t *args);
int algo_light_sensor_data_publish(servers_publish_data_args_t *args);
int algo_result_report_publish(servers_publish_data_args_t *args);
#ifdef __cplusplus
}
#endif

#endif 