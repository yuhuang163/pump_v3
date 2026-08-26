#ifndef __APP_PROTOCOL_HANDLER_H__
#define __APP_PROTOCOL_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "app_old_protocol_parser.h"

/** 旧协议上报数据类型 */
typedef enum {
    OLD_PUBLISH_DATA_POLL_REPORT,                   /// 轮询上报
    OLD_PUBLISH_DATA_PUMP_LEVEL_CHANGE = 1,      /// 泵等级变化
    OLD_PUBLISH_DATA_PUMP_SPEED_CHANGE = 2,      /// 泵速变化
    OLD_PUBLISH_DATA_PUMP_MODE_CHANGE = 3,       /// 泵模式变化
    OLD_PUBLISH_DATA_PUMP_STEP_MODE_CHANGE = 4,  /// 步骤模式变化
    OLD_PUBLISH_DATA_PUMP_STEP_LEVEL_CHANGE = 5, /// 步骤等级变化
    OLD_PUBLISH_DATA_PUMP_STEP_SPEED_CHANGE = 6, /// 步骤速率变化
    OLD_PUBLISH_DATA_PUMP_STEP_ALL_PARAM_CHANGE = 7, /// 步骤所有参数变化
    OLD_PUBLISH_DATA_BATTARNG_CHANGE = 8,         /// 电池电量变化
    OLD_PUBLISH_DATA_SYSTEM_STATUS_CHANGE = 9,    /// 系统状态变化
    OLD_PUBLISH_DATA_TASTE_POLL_REPORT = 10,    /// 体验模式轮询上报
    OLD_PUBLISH_DATA_TASTE_START_REPORT = 11,    /// 体验模式启动上报
    OLD_PUBLISH_DATA_TASTE_STOP_REPORT = 12,    /// 体验模式停止上报
    OLD_PUBLISH_DATA_TASTE_EXIT_REPORT = 13,    /// 体验模式退出上报
    OLD_PUBLISH_DATA_TASTE_MODE_CHANGE = 14,    /// 体验模式模式变化
    OLD_PUBLISH_DATA_TASTE_LEVEL_CHANGE = 15,    /// 体验模式等级变化
    OLD_PUBLISH_DATA_TASTE_SPEED_CHANGE = 16,    /// 体验模式速率变化
    OLD_PUBLISH_DATA_PUMP_CONTROL_TYPE_CHANGE = 17, /// 泵控制类型变化
    OLD_PUBLISH_DATA_SYSTEM_STATUS_RESET = 18,    /// 强制上报系统状态复位
    OLD_PUBLISH_DATA_PUMP_STATUS_FINISH = 19,    /// 强制上报泵状态结束
    OLD_PUBLISH_DATA_LIGHT_STATUS_CHANGE = 20,    /// 灯光状态变化
    OLD_PUBLISH_DATA_ID_MAX
} servers_old_protocol_publish_data_id_e;

#include "app_protocol_pump_ctrl_server.h" // IWYU pragma: keep
#include "app_protocol_sys_ctrl_server.h" // IWYU pragma: keep
#include "app_protocol_auth_server.h" // IWYU pragma: keep
#include "app_protocol_taste_ctrl_server.h" // IWYU pragma: keep

/** V3 设备异常类型（配合 APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL 首字节） */
typedef enum {
    OLD_PUBLISH_ABNORMAL_LOW_BAT_CANNOT_START = 0x01, ///< 电量不足无法启动
    OLD_PUBLISH_ABNORMAL_LOW_BATTERY = 0x02,           ///< 设备低电
    OLD_PUBLISH_ABNORMAL_LOW_BAT_SHUTDOWN_SOON = 0x03, ///< 低电即将关机
    OLD_PUBLISH_ABNORMAL_AIR_LEAK = 0x04,              ///< 设备微漏气提醒
    OLD_PUBLISH_ABNORMAL_TRAVEL_LOCK = 0x05,           ///< 进入旅行锁
    OLD_PUBLISH_ABNORMAL_ADAPTER_ERROR = 0x06,         ///< 适配器异常
    OLD_PUBLISH_ABNORMAL_GUIDE_WINDOW = 0x07,          ///< 设备处于用户引导窗口不允许启动
    OLD_PUBLISH_ABNORMAL_FAULT_WARNING = 0x08,         ///< 设备故障
    OLD_PUBLISH_ABNORMAL_RHYTHM_SWITCHED = 0x09,       ///< 已切换韵律,吸乳重新计时
    OLD_PUBLISH_ABNORMAL_PUMP_OVER_30MIN = 0x0A,       ///< 吸奶超过30分钟,建议稍作休息
    OLD_PUBLISH_ABNORMAL_STRONG_AIR_LEAK = 0x0B,       ///< 设备强漏气提醒
} old_publish_abnormal_type_e;

/** V3 设备异常状态（配合 APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL 次字节） */
typedef enum {
    OLD_PUBLISH_ABNORMAL_STATE_CLEARED = 0x00, ///< 异常解除
    OLD_PUBLISH_ABNORMAL_STATE_ACTIVE = 0x01,  ///< 异常产生
} app_old_comm_v3_abnormal_state_e;

typedef struct app_old_protocol_handler_context {
    protocol_manager_context_t *protocol_ctx;
    uint8_t encrypt_status;
    uint8_t encrypt_key;
    app_old_comm_pump_taste_ctrl taste_ctrl;
} app_old_protocol_handler_context_t;

void app_old_comm_handle_request_cmd(app_old_comm_cmd_id_e cid, uint8_t cal, uint8_t *cab);
void app_old_comm_handle_response_cmd(app_old_comm_cmd_id_e ct, uint8_t cal, void *cab);
void app_old_protocol_data_cryption(uint8_t key, uint8_t *dat, uint8_t len);
void rc4(unsigned char *key, unsigned int keyLength, unsigned char *data, unsigned int dataLength);
void app_old_protocol_ctx_init(void);
void app_old_protocol_hander_frame_builder_send(app_old_commcmd_type_e cmdtype, app_old_comm_cmd_id_e cid,
                                                uint8_t data_len, uint8_t *data);
app_old_protocol_handler_context_t *app_old_protocol_handler_ctx_get(void);
#ifdef __cplusplus
}
#endif

#endif 