#ifndef __APP_OLD_PROTOCOL_PARSER_H__
#define __APP_OLD_PROTOCOL_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

// Protocol core definitions
#define APP_OLD_COMM_SOP_FCF          0xAA55  // Frame header
#define APP_OLD_COMM_MAX_PACKET_LEN   256     // Maximum packet length (MTU=128)
#define APP_OLD_COMM_HEARTBEAT_INTERVAL 30000 // Heartbeat interval (30 seconds)
#define APP_OLD_COMM_RX_QUEUE_SIZE    512     // Receive queue size
#define APP_OLD_COMM_FRAME_TIMEOUT_MS 400     // Frame timeout (400ms)
#define BLE_PAYLOAD_CRYPTION          1       // Enable BLE payload encryption

// Command type definitions
typedef enum {
    APP_OLD_COMM_CT_REQ    = 0x00,  // Request
    APP_OLD_COMM_CT_ACK    = 0x01,  // Acknowledge
    APP_OLD_COMM_CT_NACK   = 0x02,  // Negative acknowledge
    APP_OLD_COMM_CT_NOTIFY = 0x03,  // Active notification
    APP_OLD_COMM_CT_DEVICE = 0x04   // Hardware control report
} app_old_commcmd_type_e;

// Core command ID definitions
typedef enum {
    APP_OLD_COMM_CID_HEARTBEAT = 0x00, // 心跳包

    APP_OLD_COMM_CID_TEST_DATA = 0x10, // 测试数据

    APP_OLD_COMM_CID_GET_SN = 0xA0,              // 读取设备SN
    APP_OLD_COMM_CID_SET_SN = 0xA1,              // 写入设备SN
    APP_OLD_COMM_CID_GET_MAC = 0xA2,             // 读取设备MAC
    APP_OLD_COMM_CID_SET_MAC = 0xA3,             // 写入设备MAC
    APP_OLD_COMM_CID_GET_CODE = 0xA4,            // 读取设备自定义信息
    APP_OLD_COMM_CID_SET_CODE = 0xA5,            // 写入设备自定义信息
    APP_OLD_COMM_CID_GET_SHW = 0xA6,             // 读取设备软硬件版本号
    APP_OLD_COMM_CID_GET_TK_DATA = 0xA7,         // 读取设备满奶检测数据
    APP_OLD_COMM_CID_TK_TEST_MODE_DATA = 0xA8,   // 打开或关闭满奶检模式
    APP_OLD_COMM_CID_MC11_GET_DATA = 0xA9,       // 读取MC11满奶检测数据
    APP_OLD_COMM_CID_MC11_SET_REF_DATA = 0xAA,   // 写入MC11满奶检测基准值(最小值)
    APP_OLD_COMM_CID_MC11_SET_GATE_DATA = 0xAB,  // 写入MC11满奶检测标定值(最大值)
    APP_OLD_COMM_CID_RESET_LED_TEST_MODE = 0xAC, // 重置LED测试模式
    APP_OLD_COMM_CID_READ_GATE_STATUS = 0xAD,    // 读取满奶检测状态
    APP_OLD_COMM_CID_FACTORY = 0xAF,             // 老化模式测试命令

    APP_OLD_COMM_CID_GET_STATUS = 0xB0,           // 获取吸奶器状态
    APP_OLD_COMM_CID_GET_BAND = 0xB1,             // 获取吸奶器绑定状态
    APP_OLD_COMM_CID_SET_PRIVATE_PARAM = 0xB2,    // 设置泵的运行参数
    APP_OLD_COMM_CID_GET_RECORD_NUM = 0xB3,       // 获取使用记录数量
    APP_OLD_COMM_CID_GET_RECORD_DATA = 0xB4,      // 获取使用记录内容
    APP_OLD_COMM_CID_GET_UD_ID = 0xB5,            // 获取自定义模式ID
    //APP_OLD_COMM_CID_GET_HEAT_STATE = 0xB6,       // 获取加热状态
    APP_OLD_COMM_CID_GET_TASTE_MODE_STATE = 0xB8, // 获取体验模式状态
    //APP_OLD_COMM_CID_GET_MOTOR_STATE = 0xB9,      // 获取振子状态
    APP_OLD_COMM_CID_GET_BURY_POINT_NUM = 0xBA,   // 获取埋点数量
    APP_OLD_COMM_CID_GET_BURY_POINT_DATA = 0xBB,  // 获取埋点数据
    //APP_OLD_COMM_CID_GET_FLANGE_STATUS = 0xBC,    // 获取法兰状态
    APP_OLD_COMM_CID_SET_TRAVEL_LOCK = 0xBD,      // Travel lock control
    APP_OLD_COMM_CID_GET_ABNORMAL_INFOR = 0xEC,   //获取设备异常(ACK CAL=7)

    APP_OLD_COMM_CID_CTRL_DEV = 0xC0, // 功能控制
    APP_OLD_COMM_CID_SET_BIND = 0xC1, // 设置绑定
    APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL = 0xC2, // APP 灯光控制
    APP_OLD_COMM_CID_CTRL_DEV_STATUS = 0xC3, // 设备状态控制 主要屏幕亮度，夜灯亮度
    APP_OLD_COMM_CID_GET_LIGHT_STATUS = 0xBE,  // APP 获取灯光状态

    APP_OLD_COMM_CID_USER_DEFINE = 0xC4,     // 自定义模式设置
    APP_OLD_COMM_CID_PARA_DOWNLOAD = 0xC5,   // 自定义参数下载写入flash
    APP_OLD_COMM_CID_CLEAR_CUSTOM_MODE = 0xE9, // 清除自定义模式设置参数
    APP_OLD_COMM_CID_SET_BILATERAL_PUMP_PARAM = 0xEA, // 设置泵奶档位/频次/控制类型(CAL=5)
    /// 个性化/自定义模式 ID 主动上报（Notify, CAL=4，0x00000000 表示 ID 为空）
    APP_OLD_COMM_CID_NOTIFY_CUSTOM_MODE_ID = 0xED,
    //APP_OLD_COMM_CID_SET_TK_PARA = 0xC6,     // 满奶检测参数设置
    //APP_OLD_COMM_CID_TK_FUN_CTRL = 0xC7,     // 满奶检测功能控制
    APP_OLD_COMM_CID_TASTE_CTRL = 0xC8,      // 体验模式控制
    //APP_OLD_COMM_CID_MOTOR_CTRL = 0xC9,      // 体验振子模式控制
    //APP_OLD_COMM_CID_SET_HEAT = 0xCA,        // 体验加热模式控制
    APP_OLD_COMM_CID_RESET_STATUS = 0xCF,  // 重置吸奶器状态
    
    APP_OLD_COMM_CID_NOTIFY_STATUS = 0xD0,     // 设备主动上报状态变化
    APP_OLD_COMM_CID_NOTIFY_LIGHT_STATUS = 0xDE, // 设备主动上报灯光状态
    APP_OLD_COMM_CID_NOTIFY_BIND = 0xD1,       // 设备主动上报绑定
    APP_OLD_COMM_CID_NOTIFY_DISCON = 0xD2,     // 设备强制断开BLE上报
    APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL = 0xEB, // V3设备异常状态上报(CAL=2)
    APP_OLD_COMM_CID_TASTE_REPORT = 0xD8,      // 设备主动上报体验模式状态变化
    APP_OLD_COMM_CID_BURY_POINT_REPORT = 0xDB, // 设备主动上报埋点数据

    APP_OLD_COMM_CID_GET_DEV_INFO = 0xF0,      // 获取设备信息
    APP_OLD_COMM_CID_GET_DEV_SN = 0xF1,        // 获取设备SN码
    APP_OLD_COMM_CID_SET_RTC = 0xF2,           // 设置RTC时钟
    APP_OLD_COMM_CID_GET_TRIPLE = 0xF7,        // 读取三元组，返回productID+deviceID+后8位key
    APP_OLD_COMM_CID_SET_PRODUCTID = 0xF8,     // 写入产品名称
    APP_OLD_COMM_CID_SET_DEVICEID = 0xF9,      // 写入设备号
    APP_OLD_COMM_CID_SET_ENCRYPTIONKEY = 0XFA, // 写入密钥
    //APP_OLD_COMM_CID_SET_DEVIECESIDE = 0XFB,   // 设置设备左右边
    APP_OLD_COMM_CID_SYS_CTRL = 0xFC,          // 系统控制,设备的开关重启,ota完成，ota复位
    APP_OLD_COMM_CID_GET_ENCRYPT = 0xFD,       // 获取加密信息
    //APP_OLD_COMM_CID_TEST_CTRL = 0xFE,         // 实验参数控制

    // 测试命令
    //BLE_CMD_GET_VOLT = 0XE0,          // 获取电池电压
    //BLE_CMD_GET_BAT = 0XE1,           // 获取电池电量
    //BLE_CMD_GET_FLANGER = 0XE2,       // 获取法兰温度
    //BLE_CMD_UD_MODE_SET_HEAT = 0XE7,  // 自定义模式设置加热参数
    //BLE_CMD_UD_MODE_SET_SHAKE = 0XE8, // 自定义模式设置振子参数

} app_old_comm_cmd_id_e;

// Protocol parsing result enumeration
typedef enum {
    APP_OLD_COMM_PARSE_SUCCESS = 0,       // Parse successful
    APP_OLD_COMM_PARSE_ERR_HEADER,        // Header error
    APP_OLD_COMM_PARSE_ERR_CHECKSUM,      // Checksum error
    APP_OLD_COMM_PARSE_ERR_LENGTH,        // Length error
    APP_OLD_COMM_PARSE_ERR_CMD_UNKNOWN,   // Unknown command
    APP_OLD_COMM_PARSE_ERR_INCOMPLETE,    // Incomplete data
    APP_OLD_COMM_PARSE_ERR_TIMEOUT        // Frame timeout
} app_old_comm_par_re_result_e;

// Ring queue structure
typedef struct {
    uint8_t buffer[APP_OLD_COMM_RX_QUEUE_SIZE];  // Queue buffer
    uint16_t head;                           // Head index
    uint16_t tail;                           // Tail index
    uint16_t len;                            // Current queue length
    uint32_t last_recv_time;                 // Last received data timestamp (ms)
} app_old_comm_ring_queue_t;

typedef struct {
    uint16_t sof;               ///< Start Of Frame: 0xAA55
    uint8_t ct;                 ///<  指令类型节点
    uint8_t cid;                 ///< cmd节点
    uint8_t cal;                 ///< Payload长度
    uint8_t *cab;                 ///< Payload数据
    uint8_t checksum;            ///< CRC8校验和，覆盖Header和Payload域
} old_protocol_frame_node_t;

/**
 * @brief 组包完成回调函数类型
 * @param frame_data 完整的帧数据
 * @param frame_len 帧数据长度
 * @param user_data 用户自定义数据
 */
typedef void (*app_old_protocol_frame_callback_t)(uint8_t *frame_data, uint16_t frame_len);

/**
 * @brief 组包接口 - 处理分片数据并组装成完整帧
 * @param data 接收到的数据片段
 * @param len 数据片段长度
 * @param callback 当组装完成时的回调函数
 * @param user_data 用户自定义数据，会传递给回调函数
 * @return 0-成功处理, 1-需要更多数据, <0-错误
 */
int app_old_protocol_assemble_packet(uint8_t *data, uint16_t len, app_old_protocol_frame_callback_t callback);

int app_old_protocol_frame_deserialize(uint8_t *frame_buf, uint16_t len, old_protocol_frame_node_t *frame);
int app_old_protocol_frame_builder_send(app_old_commcmd_type_e cmdtype, app_old_comm_cmd_id_e cid, uint8_t data_len, uint8_t *data);

const char* app_old_comm_get_cmd_id_str(app_old_comm_cmd_id_e cid);
const char* app_old_comm_get_cmd_type_str(app_old_commcmd_type_e ct);
#ifdef __cplusplus
}
#endif

#endif 