#ifndef __APP_PROTOCOL_WINDOW_MSG_H__
#define __APP_PROTOCOL_WINDOW_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

/**
 * App 控制 → 窗口消息通道。
 *
 * App 下发泵控制（模式切换 / 启停 / 档位 / speed）后，由本模块通过
 * wm->post_window_event 向当前窗口（home / pump）投递异步事件，通知 UI 刷新。
 */

/// 各窗口消息开关：0 = 屏蔽（不实际发消息），1 = 发送。后续需要时单独改为 1。
#ifndef APP_WINDOW_MSG_MODE_ENABLE
#define APP_WINDOW_MSG_MODE_ENABLE 1
#endif
#ifndef APP_WINDOW_MSG_LEVEL_ENABLE
#define APP_WINDOW_MSG_LEVEL_ENABLE 0
#endif
#ifndef APP_WINDOW_MSG_SPEED_ENABLE
#define APP_WINDOW_MSG_SPEED_ENABLE 0
#endif
#ifndef APP_WINDOW_MSG_STATUS_ENABLE
#define APP_WINDOW_MSG_STATUS_ENABLE 0
#endif

/// 模式切换：mode 为 breast_pump_mode_e；name 为模式名（自定义模式带名字，其余传 NULL）
void app_window_msg_pump_mode(uint8_t mode, const char *name);

/// 档位调节：left/right 为左右档位，side 为 WINDOW_EVT_LEVEL_SIDE_*
void app_window_msg_pump_level(uint8_t left, uint8_t right, uint8_t side);

/// 速度调节：speed 为速度档位
void app_window_msg_pump_speed(uint8_t speed);

/// 泵奶状态变化（启停）：status 为 breast_pump_work_status_e
void app_window_msg_pump_status(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif /* __APP_PROTOCOL_WINDOW_MSG_H__ */
