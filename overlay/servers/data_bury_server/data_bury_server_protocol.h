#ifndef __DATA_BURY_SERVER_PROTOCOL_H__
#define __DATA_BURY_SERVER_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_data_bury_protocol_defs.h"

void data_bury_server_protocol_init(void);

/** 链路层帧解析完成后调用：Device Management 下行命令处理 + Data Service 接收日志 */
void data_bury_server_protocol_on_frame(const app_data_bury_protocol_frame_node_t *frame);

/**
 * 推送一条埋点事件（packed 载荷），TLV 组包后经 BLE 发送
 */
int data_bury_server_push_event(uint8_t cmd_id, const void *packed, uint16_t packed_len);

/** TLV 组包并发送 packed 载荷 */
int data_bury_server_send_packed(uint8_t svc_id, uint8_t cmd_id, const void *packed, uint16_t packed_len);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_SERVER_PROTOCOL_H__ */
