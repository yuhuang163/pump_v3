#ifndef __DATA_BURY_SERVER_PROTOCOL_H__
#define __DATA_BURY_SERVER_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_data_bury_protocol_defs.h"

void data_bury_server_protocol_init(void);

/** 链路层帧解析完成后调用：Data Service 接收日志（设备侧仅上报，不做 TLV 解包） */
void data_bury_server_protocol_on_frame(const app_data_bury_protocol_frame_node_t *frame);

/**
 * 推送一条埋点事件（packed 载荷）
 * BLE 已连接则 TLV 组包发送；否则写入 RAM 环
 */
int data_bury_server_push_event(uint8_t cmd_id, const void *packed, uint16_t packed_len);

/** 将 RAM 环中 pending 事件逐条 TLV 组包发送，成功一条 pop 一条；返回本次发送条数 */
int data_bury_server_flush_pending(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_SERVER_PROTOCOL_H__ */
