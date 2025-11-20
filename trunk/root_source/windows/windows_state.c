/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_map_define.h"

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
static bool is_on_window_dispatch(window_manager_context_t *wm)
{
    EXCEPTION_CHECK_BOOL(wm);
    return wm->priv.window_dispatch_thd.is_running > 0;
}

static bool is_current_window(window_manager_context_t *wm, uint8_t window_id)
{
    EXCEPTION_CHECK_BOOL(wm);
    return wm->get_current_window_id() == window_id ? true : false;
} 

static bool is_the_same_window(window_manager_context_t *wm, uint8_t new_window_id) 
{
    EXCEPTION_CHECK_BOOL(wm);
    return wm->get_current_window_id() == new_window_id ? true : false;
}

static bool is_valid_window_id(uint8_t window_id)
{
    if (window_id >= ID_WINDOW_MAX || window_id == ID_WINDOW_INVALID) {
        return false;
    }
    return window_id;
}

static bool is_window_guidance_deployed(void)
{
    storage_manager_context_t *st_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    int guide_deployed = 0;
    st_ctx->get_store_ram_val(STORE_ID_SYS_GUIDE_DEPLOYED, 0, 0, &guide_deployed);
    return guide_deployed ? 1 : 0;
}

static void set_window_guidance_deployed_flag(void)
{
    storage_manager_context_t *st_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    st_ctx->put_store_ram_val(STORE_ID_SYS_GUIDE_DEPLOYED, 1, 0, NULL);
}

static bool is_window_system_is_ready(window_manager_context_t *wm)
{
    EXCEPTION_CHECK_BOOL(wm);
    return wm->priv.system_is_ready > 0 ? true : false;
}

static void window_boot_completed(window_manager_context_t *wm)
{
    EXCEPTION_CHECK_VOID(wm);
    if (!wm->priv.system_is_ready) {
        wm->priv.system_is_ready = 1;
        SEND_BROADCAST_ARGS0(ID_BROADCAST_SYS_BOOT_COMPLETED);
    }
}

static void set_window_private_context(window_manager_context_t *wm, uint8_t window_id, void *context)
{
    EXCEPTION_CHECK_VOID(wm);
    if (window_id < ID_WINDOW_MAX) {
        wm->priv.window_private[window_id] = context;
        return;
    }
    LOGE("error window_id: %d", window_id);
}

static void *get_window_private_context(window_manager_context_t *wm, uint8_t window_id)
{
    EXCEPTION_CHECK_POINT(wm);
    if (window_id < ID_WINDOW_MAX) {
        return wm->priv.window_private[window_id];
    }
    LOGE("error window_id: %d", window_id);
    return NULL;
}

static const void *get_window_entry_extra(window_manager_context_t *wm)
{
    EXCEPTION_CHECK_POINT(wm);
    return (const void *)&wm->priv.new_msg.union_body.extra;
}

static void set_system_is_restting_device(window_manager_context_t *wm, bool val)
{
    LOG_WM("is_restting_device: %d", val);
    wm->priv.is_restting_device = val;
}

static bool get_system_is_restting_device(window_manager_context_t *wm)
{
    EXCEPTION_CHECK_BOOL(wm);
    return wm->priv.is_restting_device > 0 ? true : false;
}

static int set_window_switch_forbid(window_manager_context_t *wm, int forbid) 
{
    EXCEPTION_CHECK_INT(wm);
    LOG_WM("forbid: %d", forbid);
    wm->priv.window_switch_forbid = forbid;
    return 0;
}

static void clear_window_go_back_history_flag(window_manager_context_t *wm)
{
    EXCEPTION_CHECK_VOID(wm);
    if (wm->priv.go_back_history_flag) {
        wm->priv.go_back_history_flag = 0;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_window_state_module(window_manager_context_t *wm)
{
    wm->system_is_ready = is_window_system_is_ready;
    wm->window_boot_completed = window_boot_completed;
    wm->is_on_window_dispatch = is_on_window_dispatch;
    wm->is_current_window = is_current_window;
    wm->is_the_same_window = is_the_same_window;
    wm->is_valid_window_id = is_valid_window_id;
    wm->is_window_guidance_deployed = is_window_guidance_deployed;
    wm->set_window_private_context = set_window_private_context;
    wm->get_window_private_context = get_window_private_context;
    wm->get_window_entry_extra = get_window_entry_extra;
    wm->set_system_is_restting_device = set_system_is_restting_device;
    wm->get_system_is_restting_device = get_system_is_restting_device;
    wm->set_window_switch_forbid = set_window_switch_forbid;
    wm->clear_window_go_back_history_flag = clear_window_go_back_history_flag;
}