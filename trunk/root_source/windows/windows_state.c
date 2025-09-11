/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_internal_api.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static window_manager_context_t *thiz = NULL;

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
void set_is_playing_rt_ani(bool flag)
{
    thiz->priv.is_playing_realtime_ani = flag;
}

const char *to_window_name(uint8_t window_id) 
{
    switch (window_id) {
        case ID_WINDOW_DUMMY:
            return "dummy";
        case ID_WINDOW_LOGO:
            return "logo";
        case ID_WINDOW_GUIDE:
            return "guide";
        case ID_WINDOW_CHARGE:
            return "charging";
        case ID_WINDOW_BATTERY:
            return "battery";
        case ID_WINDOW_BREATH_PUMP:
            return "breath_pump";
        case ID_WINDOW_SETTINGS:
            return "setting";
        case ID_WINDOW_OTA_UPGRADE:
            return "ota_upgrade";
        case ID_WINDOW_FACTORY_MODE:
            return "factory_mode";
        case ID_WINDOW_POPUP_WINDOW:
            return "popup_window";
        case ID_WINDOW_AIRPLANE_MODE:
            return "airplane_mode";
        default:
            return "unknow";
    }
}

bool is_window_backlight_on(void)
{
    return thiz->priv.m_backlight_val > 0 ? true : false;
}

bool is_on_window_dispatch(void)
{
    return thiz->priv.window_dispatch_thd.is_running > 0;
}

bool is_current_window(uint8_t window_id)
{
    return thiz->get_current_window_id() == window_id ? true : false;
} 

bool is_the_same_window(uint8_t new_window_id) 
{
    return thiz->get_current_window_id() == new_window_id ? true : false;
}

bool is_valid_window_id(uint8_t window_id)
{
    if (window_id > ID_WINDOW_BEGIN && window_id < ID_WINDOW_MAX) {
        return true;
    }
    return false;
}

/// 是否完成首次开机部署.
bool is_window_deployed(void)
{
    storage_context_t *st_ctx = (storage_context_t *)find_module(ID_MODULE_STORAGE);
    return st_ctx->get_store_ram_val(STORE_ID_SYS_GUIDE_DEPLOYED) ? 1 : 0;
}

/// 设置首次开机部署标志.
void set_window_deployed_flag(void)
{
    storage_context_t *st_ctx = (storage_context_t *)find_module(ID_MODULE_STORAGE);
    st_ctx->put_store_ram_val(STORE_ID_SYS_GUIDE_DEPLOYED, 1);
}

bool window_system_is_ready(void)
{
    return thiz->priv.system_is_ready;
}

void window_boot_completed(void)
{
    if (!thiz->priv.system_is_ready) {
        thiz->priv.system_is_ready = 1;
        LOGW("**** congratulations! send ID_BROADCAST_SYS_BOOT_COMPLETED ****");
        SEND_BROADCAST_ARGS0(ID_BROADCAST_SYS_BOOT_COMPLETED);
    }
}

/// 记录窗口上下文指针
void set_window_private_context(uint8_t window_id, void *context)
{
    if (window_id < ID_WINDOW_MAX) {
        thiz->priv.window_private[window_id] = context;
        return;
    }
    LOGE("error window_id: %d", window_id);
}

/// 获取窗口上下文指针
void *get_window_private_context(uint8_t window_id)
{
    if (window_id < ID_WINDOW_MAX) {
        return thiz->priv.window_private[window_id];
    }
    LOGE("error window_id: %d", window_id);

    return NULL;
}

/// 进入新窗口带入的参数.
const void *get_window_entry_extra(void)
{
    return (const void *)&thiz->priv.new_msg.union_body.extra;
}

void set_window_backlight(uint16_t val)
{
    DEVICE_IOCTL_ARGS_NO_RET(thiz->priv.panel_dev, OP_CODE_PANEL_SET_BL, &val);
}

void set_system_is_restting_device(bool val)
{
    LOG_WM("is_restting_device: %d", val);
    thiz->priv.is_restting_device = val;
}

uint8_t get_system_is_restting_device(void)
{
    return thiz->priv.is_restting_device;
}

int set_window_switch_forbid(int forbid) 
{
    LOG_WM("forbid: %d", forbid);
    thiz->priv.window_switch_forbid = forbid;
    return 0;
}

void clear_window_go_back_history_flag(void)
{
    if (thiz->priv.go_back_history_flag) {
        thiz->priv.go_back_history_flag = 0;
    }
}

void on_perpare_window_state_init(window_manager_context_t *wm)
{
    thiz = wm;
}