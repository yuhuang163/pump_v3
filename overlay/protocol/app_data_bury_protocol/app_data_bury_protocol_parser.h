#ifndef __APP_DATA_BURY_PROTOCOL_PARSER_H__
#define __APP_DATA_BURY_PROTOCOL_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_data_bury_protocol_defs.h"

/*============================================================================
 * 组包完成回调：传递完整帧（指向内部环形/静态缓冲区，回调内需立即解析）
 *============================================================================*/
typedef void (*app_data_bury_protocol_frame_callback_t)(const uint8_t *frame_data, uint16_t frame_len);

void app_data_bury_protocol_init(void);

/*
 * @brief 分片重组 + 组帧解析入口
 * @param data BLE 写入/通知的字节流片段（可能是 MTU 分片）
 * @param len 片段长度
 * @param callback 当完整帧到达时回调
 */
int app_data_bury_protocol_assemble_packet(const uint8_t *data, uint16_t len,
                                            app_data_bury_protocol_frame_callback_t callback);

/*
 * @brief 完整帧反序列化（解析出 svc/cmd/payload 等）
 * @return 0 成功；非 0 为错误（frame node 中也会标识 crc_ok）
 */
int app_data_bury_protocol_frame_deserialize(const uint8_t *buffer, uint16_t buffer_size,
                                               app_data_bury_protocol_frame_node_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* __APP_DATA_BURY_PROTOCOL_PARSER_H__ */

