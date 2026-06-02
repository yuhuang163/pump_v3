#ifndef __APP_DATA_BURY_PROTOCOL_BUILDER_H__
#define __APP_DATA_BURY_PROTOCOL_BUILDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_data_bury_protocol_defs.h"

/*
 * @brief 发送应用层数据：自动组装链路层 + ACK 置位
 * @param svc_id Service ID (0x01/0x07 等)
 * @param cmd_id Command ID
 * @param payload 业务 TLV bytes（不包含 svc_id/cmd_id）
 * @param payload_len 业务 TLV 长度
 */
int app_data_bury_protocol_frame_builder_send(uint8_t svc_id, uint8_t cmd_id,
                                              const uint8_t *payload, uint16_t payload_len);

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

