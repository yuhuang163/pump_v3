#ifndef __APP_DATA_BURY_PROTOCOL_BUILDER_H__
#define __APP_DATA_BURY_PROTOCOL_BUILDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_data_bury_protocol_defs.h"

/*
 * @brief 单帧发送（默认）：SVC+CMD+TLV 整包装入一条 FSN_COMPLETE 链路帧
 * @return 0 成功；-1 参数/长度非法；-2 BLE 不可用；-3 未连接；-4 组帧失败
 */
int app_data_bury_protocol_frame_builder_send(uint8_t svc_id, uint8_t cmd_id,
                                              const uint8_t *payload, uint16_t payload_len);

/*
 * @brief MFS 分帧发送：应用层按 MFS(默认64B) 切 START/MID/END 多链路帧
 * @param mfs 分片阈值，0 表示使用 APP_DATA_BURY_PROTOCOL_MFS_DEFAULT
 */
int app_data_bury_protocol_frame_builder_send_split(uint8_t svc_id, uint8_t cmd_id,
                                                    const uint8_t *payload, uint16_t payload_len,
                                                    uint16_t mfs);

/*============================================================================
 * TLV 编码辅助（Type(1) + Length(1) + Value）- 大端/端序由业务自行定义
 *============================================================================*/
uint16_t app_data_bury_protocol_tlv_put_u8(uint8_t *buf, uint16_t cap, uint8_t type, uint8_t val);
uint16_t app_data_bury_protocol_tlv_put_u16(uint8_t *buf, uint16_t cap, uint8_t type, uint16_t val);
uint16_t app_data_bury_protocol_tlv_put_u32(uint8_t *buf, uint16_t cap, uint8_t type, uint32_t val);
uint16_t app_data_bury_protocol_tlv_put_raw(uint8_t *buf, uint16_t cap, uint8_t type, const uint8_t *val, uint8_t vlen);

#ifdef __cplusplus
}
#endif

#endif /* __APP_DATA_BURY_PROTOCOL_BUILDER_H__ */

