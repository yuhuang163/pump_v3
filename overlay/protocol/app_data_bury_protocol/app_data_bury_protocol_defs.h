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
#define APP_DATA_BURY_PROTOCOL_MFS_DEFAULT       64

/* 应用层最大包长（svc+cmd+payload+TLV 等） */
#define APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX  500

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

/* Data Service Command ID
 * 具体业务 EVT_001~EVT_019 会在上层协议解析管理模块使用；
 * 这里先保留常量占位（避免耦合业务结构体）。
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

/* TLV Type */
#define APP_DATA_BURY_PROTOCOL_TLV_TYPE_ERROR        127 /* 0x7F */
#define APP_DATA_BURY_PROTOCOL_TLV_TYPE_EVENT_DATA   0x00
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

