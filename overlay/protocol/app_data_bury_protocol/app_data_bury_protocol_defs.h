#ifndef __APP_DATA_BURY_PROTOCOL_DEFS_H__
#define __APP_DATA_BURY_PROTOCOL_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*============================================================================
 * Link layer (协议 v1.0.x 使用 0x5A SOF + CRC16-CCITT-FALSE)
 *============================================================================*/
#define APP_DATA_BURY_PROTOCOL_SOF                 0x5A

/* SOF(1) + Length(2) + Control(1) + [FSN(1)] */
#define APP_DATA_BURY_PROTOCOL_HEADER_MIN        5

/* CRC16 */
#define APP_DATA_BURY_PROTOCOL_FOOTER_SIZE        2

/* 协议默认分帧能力（对应 MFS/单帧阈值） */
#define APP_DATA_BURY_PROTOCOL_MFS_DEFAULT       128

/* 应用层最大包长（svc+cmd+payload+TLV 等） */
#define APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX  247

/* Control field: Bit2=ACK, Bit1-0=FSN */
#define APP_DATA_BURY_PROTOCOL_CTRL_FSN_MASK     0x03
#define APP_DATA_BURY_PROTOCOL_CTRL_FSN_COMPLETE 0x00
#define APP_DATA_BURY_PROTOCOL_CTRL_FSN_START    0x01
#define APP_DATA_BURY_PROTOCOL_CTRL_FSN_MID      0x02
#define APP_DATA_BURY_PROTOCOL_CTRL_FSN_END      0x03
#define APP_DATA_BURY_PROTOCOL_CTRL_ACK          0x04

/*============================================================================
 * Application layer
 *============================================================================*/
/* Service ID */
#define APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT  0x01
#define APP_DATA_BURY_PROTOCOL_SVC_DATA         0x07

/* Device Management Service (0x01) Command ID */
#define APP_BURY_DEV_MGMT_CMD_CONN_PARAM_NEGOTIATE  0x01
#define APP_BURY_DEV_MGMT_CMD_SET_DEVICE_TIME       0x02

/* 连接参数协商 TLV Type */
#define APP_BURY_DEV_TLV_PROT_VERSION       0x01
#define APP_BURY_DEV_TLV_MAX_FRAME_SIZE     0x02
#define APP_BURY_DEV_TLV_MTU                0x03
#define APP_BURY_DEV_TLV_TX_INTERVAL        0x04
#define APP_BURY_DEV_TLV_ARQ_MODE           0x05
#define APP_BURY_DEV_TLV_ACK_TIMEOUT_MS     0x06
#define APP_BURY_DEV_TLV_MAX_RETRANSMIT     0x07
#define APP_BURY_DEV_TLV_RANDOM_VALUE       0x08

/* 设置设备日期时间 TLV Type */
#define APP_BURY_DEV_TLV_TIME               0x01
#define APP_BURY_DEV_TLV_TIME_OFFSET        0x02

/* 通用错误码（wire 编码 0x0010xxxx，与链路层 ACK 一致） */
#define APP_DATA_BURY_PROTOCOL_ERROR_SUCCESS    0x00100000u
#define APP_DATA_BURY_PROTOCOL_ERROR_UNKNOWN    0x00100001u
#define APP_DATA_BURY_PROTOCOL_ERROR_SVC        0x00100002u
#define APP_DATA_BURY_PROTOCOL_ERROR_CMD        0x00100003u
#define APP_DATA_BURY_PROTOCOL_ERROR_PARAM      0x00100007u

/* Data Service Command ID（EVT_001~EVT_019）
 * 对外上报：servers_publish_data_id_e + data_bury_server_data_publish
 * 见 data_bury_protocol_server_client.h / data_bury_publish_protocol.h
 */
#define APP_BURY_CMD_PUMP_STARTUP           0x01
#define APP_BURY_CMD_PUMP_SHUTDOWN          0x02
#define APP_BURY_CMD_BT_CONNECT_EVENT       0x03
#define APP_BURY_CMD_PUMP_SESSION_START     0x04
#define APP_BURY_CMD_PUMP_SESSION_END       0x05
#define APP_BURY_CMD_PUMP_MANUAL_ADJUST     0x06
#define APP_BURY_CMD_PUMP_USER_EVENT        0x07
#define APP_BURY_CMD_MILK_EJECTION          0x08
#define APP_BURY_CMD_BAT_CHARGE_START       0x09
#define APP_BURY_CMD_BAT_CHARGE_END         0x0A
#define APP_BURY_CMD_EXCEPTION              0x0B
#define APP_BURY_CMD_FIRMWARE_UPDATE        0x0C
#define APP_BURY_CMD_OVERFLOW               0x0D
#define APP_BURY_CMD_MILK_VOLUME            0x0E
#define APP_BURY_CMD_HEAT_SESSION_START     0x0F
#define APP_BURY_CMD_HEAT_SESSION_END       0x10
#define APP_BURY_CMD_MASSAGE_SESSION_START  0x11
#define APP_BURY_CMD_MASSAGE_SESSION_END    0x12
#define APP_BURY_CMD_WEAR_DETECTION         0x13

/* TLV header: Type(1) + Length(1) */
#define APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE     2

/* TLV Type（v1.0.2：业务字段从 0x00 device_side 起编；127=通用错误码，非 pump_side） */
#define APP_DATA_BURY_PROTOCOL_TLV_TYPE_DEVICE_SIDE  0x00
#define APP_DATA_BURY_PROTOCOL_TLV_TYPE_ERROR        127
#define APP_DATA_BURY_PROTOCOL_TLV_TYPE_EVENT_CONT   0x01

/*============================================================================
 * Frame node (反序列化结果)
 *============================================================================*/
typedef struct {
    uint8_t  sof;              /* =0x5A */
    uint8_t  ctrl;             /* Control byte */
    uint8_t  fsn;              /* 有 FSN 时才有效 */
    uint8_t  svc_id;           /* Service ID */
    uint8_t  cmd_id;           /* Command ID */

    const uint8_t *payload;   /* payload: TLV bytes, 指向传入 buffer 内部 */
    uint16_t payload_len;

    uint8_t  ack_requested;   /* ctrl ACK bit */
    uint8_t  fsn_present;      /* ctrl FSN != COMPLETE 时为 1 */
    uint8_t  crc_ok;          /* CRC16 校验结果 */
} app_data_bury_protocol_frame_node_t;

#ifdef __cplusplus
}
#endif

#endif /* __APP_DATA_BURY_PROTOCOL_DEFS_H__ */

