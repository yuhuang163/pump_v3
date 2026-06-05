#ifndef __DATA_BURY_TLVS_PROTOCOL_H__
#define __DATA_BURY_TLVS_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** TLV 节点，对齐 comm_protocol_tlv_node_t */
typedef struct {
    uint8_t        type;
    uint8_t        length;
    const uint8_t *value;
} data_bury_tlv_node_t;

typedef struct {
    uint8_t *buf;
    uint16_t cap;
    uint16_t len;
} data_bury_tlv_builder_t;

void data_bury_tlv_builder_init(data_bury_tlv_builder_t *builder, uint8_t *buf, uint16_t cap);
int data_bury_tlv_builder_add(data_bury_tlv_builder_t *builder, const data_bury_tlv_node_t *node);
uint16_t data_bury_tlv_builder_len(const data_bury_tlv_builder_t *builder);

/** packed(LE) -> wire TLV(BE)，成功返回 TLV 字节数，失败返回 0 */
uint16_t data_bury_tlv_packed_to_wire(uint8_t cmd_id, const uint8_t *packed, uint16_t packed_len,
                                      uint8_t *out, uint16_t out_cap);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_TLVS_PROTOCOL_H__ */
