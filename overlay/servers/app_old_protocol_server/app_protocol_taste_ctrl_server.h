#ifndef __APP_PROTOCOL_TASTE_CTRL_SERVER_H__
#define __APP_PROTOCOL_TASTE_CTRL_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

/// 体验模式控制
typedef struct {
    uint8_t taste_status_ctrl; /// 0:停止 1:开始
    uint8_t taste_pump_mode; /// 1:刺激模式 2:吸奶模式 3:混合模式 4:停止
    uint8_t taste_pump_level; /// 0:1档 1:2档 2:3档 3:4档 4:5档 5:6档 6:7档 7:8档 8:9档 9:10档 10:11档 11:12档 12:13档 13:14档 14:15档
    uint8_t taste_pump_speed; /// 1:低频 2:中频 3:高频
} app_old_comm_pump_taste_ctrl;

#ifdef __cplusplus
}
#endif

#endif 

