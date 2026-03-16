#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_broadcast_to_window.h"
#include "broadcast_manager_api.h"
/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void app_broadcast_pump_mode_to_window(uint8_t mode, app_broadcast_l_r_ctrl_e lr_ct)
{
    app_broadcast_pump_mode data;
    data.lr_ct = lr_ct;
    data.pump_mode = mode;
    /// 广播模式切换
    /// SEND_BROADCAST_ARGS4_EXTRA(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_MODE_CHANGE, &data,
    ///                           sizeof(data));
}

void app_broadcast_pump_level_to_window(uint8_t level, app_broadcast_l_r_ctrl_e lr_ct)
{
    app_broadcast_pump_level data;
    data.lr_ct = lr_ct;
    data.pump_level = level;
    /// 广播泵等级变化
    /// SEND_BROADCAST_ARGS4_EXTRA(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_PUMP_LEVEL_CHANGE, &data,
    ///                           sizeof(data));
}

void app_broadcast_pump_speed_to_window(uint8_t speed, app_broadcast_l_r_ctrl_e lr_ct)
{
    app_broadcast_pump_speed data;
    data.lr_ct = lr_ct;
    data.pump_speed = speed;
    /// 广播泵速变化
    /// SEND_BROADCAST_ARGS4_EXTRA(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_PUMP_SPEED_CHANGE, &data,
    ///                           sizeof(data));
}

void app_broadcast_pump_status_to_window(uint8_t status, app_broadcast_l_r_ctrl_e lr_ct)
{
    app_broadcast_pump_status data;
    data.lr_ct = lr_ct;
    data.pump_status = status;
    /// 广播状态变化
    /// SEND_BROADCAST_ARGS4_EXTRA(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_STATE_CHANGE, &data,
    ///                           sizeof(data));
}
