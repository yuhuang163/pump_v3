/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_window_msg.h"

#if APP_WINDOW_MSG_MODE_ENABLE || APP_WINDOW_MSG_LEVEL_ENABLE || \
    APP_WINDOW_MSG_SPEED_ENABLE || APP_WINDOW_MSG_STATUS_ENABLE

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
/// 目标窗口：home / pump 都接收，其它窗口返回 INVALID 忽略
static uint8_t app_window_msg_get_target_window(window_manager_context_t *wm)
{
    if (wm->is_current_window(wm, ID_WINDOW_HOME)) {
        return ID_WINDOW_HOME;
    }
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        return ID_WINDOW_PUMP;
    }
    return ID_WINDOW_INVALID;
}

#endif /* 任一消息开关开启 */

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if APP_WINDOW_MSG_MODE_ENABLE
void app_window_msg_pump_mode(uint8_t mode, const char *name)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    window_evt_mode_t *data;
    uint8_t window_id;

    if (!wm || !wm->post_window_event) {
        return;
    }
    window_id = app_window_msg_get_target_window(wm);
    if (window_id == ID_WINDOW_INVALID) {
        return;
    }
    data = (window_evt_mode_t *)u_malloz(sizeof(*data));
    if (!data) {
        return;
    }
    data->mode = mode;
    if (name) {
        strncpy(data->name, name, sizeof(data->name) - 1);
        data->name[sizeof(data->name) - 1] = '\0';
    }
    wm->post_window_event(__func__, window_id, WINDOW_ASYNC_EVT_MODE_SWITCH,
        data, WINDOW_EVT_FLAG_FREE_USER_DATA);
}
#else
void app_window_msg_pump_mode(uint8_t mode, const char *name)
{
    (void)mode;
    (void)name;
}
#endif /* APP_WINDOW_MSG_MODE_ENABLE */

#if APP_WINDOW_MSG_LEVEL_ENABLE
void app_window_msg_pump_level(uint8_t left, uint8_t right, uint8_t side)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    window_evt_level_t *data;
    uint8_t window_id;

    if (!wm || !wm->post_window_event) {
        return;
    }
    window_id = app_window_msg_get_target_window(wm);
    if (window_id == ID_WINDOW_INVALID) {
        return;
    }
    data = (window_evt_level_t *)u_malloz(sizeof(*data));
    if (!data) {
        return;
    }
    data->level[0] = left;
    data->level[1] = right;
    data->side = side;
    wm->post_window_event(__func__, window_id, WINDOW_ASYNC_EVT_LEVEL_ADJUST,
        data, WINDOW_EVT_FLAG_FREE_USER_DATA);
}
#else
void app_window_msg_pump_level(uint8_t left, uint8_t right, uint8_t side)
{
    (void)left;
    (void)right;
    (void)side;
}
#endif /* APP_WINDOW_MSG_LEVEL_ENABLE */

#if APP_WINDOW_MSG_SPEED_ENABLE
void app_window_msg_pump_speed(uint8_t speed)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    window_evt_speed_t *data;
    uint8_t window_id;

    if (!wm || !wm->post_window_event) {
        return;
    }
    window_id = app_window_msg_get_target_window(wm);
    if (window_id == ID_WINDOW_INVALID) {
        return;
    }
    data = (window_evt_speed_t *)u_malloz(sizeof(*data));
    if (!data) {
        return;
    }
    data->speed = speed;
    wm->post_window_event(__func__, window_id, WINDOW_ASYNC_EVT_SPEED_ADJUST,
        data, WINDOW_EVT_FLAG_FREE_USER_DATA);
}
#else
void app_window_msg_pump_speed(uint8_t speed)
{
    (void)speed;
}
#endif /* APP_WINDOW_MSG_SPEED_ENABLE */

#if APP_WINDOW_MSG_STATUS_ENABLE
void app_window_msg_pump_status(uint8_t status)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    window_evt_status_t *data;
    uint8_t window_id;

    if (!wm || !wm->post_window_event) {
        return;
    }
    window_id = app_window_msg_get_target_window(wm);
    if (window_id == ID_WINDOW_INVALID) {
        return;
    }
    data = (window_evt_status_t *)u_malloz(sizeof(*data));
    if (!data) {
        return;
    }
    data->status = status;
    wm->post_window_event(__func__, window_id, WINDOW_ASYNC_EVT_PUMP_STATUS_CHANGE,
        data, WINDOW_EVT_FLAG_FREE_USER_DATA);
}
#else
void app_window_msg_pump_status(uint8_t status)
{
    (void)status;
}
#endif /* APP_WINDOW_MSG_STATUS_ENABLE */
