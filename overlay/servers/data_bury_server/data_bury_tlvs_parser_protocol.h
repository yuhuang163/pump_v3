#ifndef __DATA_BURY_TLVS_PARSER_PROTOCOL_H__
#define __DATA_BURY_TLVS_PARSER_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "app_data_bury_protocol_defs.h"

void data_bury_tlvs_parser_protocol_init(void);

/** 处理 Device Management Service (0x01) 下行帧 TLV */
void data_bury_tlvs_parser_handle_frame(const app_data_bury_protocol_frame_node_t *frame);

/** 连接参数协商后的 MFS，供埋点上报分帧使用 */
uint16_t data_bury_tlvs_parser_get_negotiated_mfs(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_TLVS_PARSER_PROTOCOL_H__ */
