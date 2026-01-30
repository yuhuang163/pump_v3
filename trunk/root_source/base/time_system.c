
/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" 

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static time_system_t *s_time_system = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void on_time_receive_internal(broadcast_mask_id_t id, void *context, int power_state, 
                    int wakeup_reason, broadcast_priv_data_t *pdata)
{
    time_system_t *ts = context;
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(ctx);
    if (id.code == ID_BROADCAST_SYS_STATE_CHANGE) {
        if (power_state == SYSTEM_BROADCAST_POWER_OFF) {
            LOG_TIME("=>shutdown save timestamp: %d", ts->priv.timestamp);
            ctx->put_store_val(STORE_ID_SYS_UTC_TIMESTAMP, ts->priv.timestamp, 0, NULL);
            return;
        }
    }
}

static int time_system_init(void *ptr)
{
    time_system_t *ts = ptr;
    uint32_t timestamp = 0;
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (ctx && ctx->get_store_val) {
        ctx->get_store_val(STORE_ID_SYS_UTC_TIMESTAMP, 0, 0, &timestamp);
    }

    LOGW("timestamp: %u", timestamp);

    /// UTC时间默认
    if (timestamp < PRODUCT_BUILD_TIMESTAMP) {
        timestamp = PRODUCT_BUILD_TIMESTAMP;
    }
    
    /// 设置UTC时间
    platform_set_utc_time(timestamp, 0);

    /// 广播监听PM状态改变.
    ts->priv.time_bcd = u_malloz(sizeof(broadcast_obj_t));
    ts->priv.time_bcd->context = ts;
    ts->priv.time_bcd->name = "time_system";
    ts->priv.time_bcd->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ts->priv.time_bcd->id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
    ts->priv.time_bcd->receiver_cb = on_time_receive_internal;
    register_broadcast_receiver(ts->priv.time_bcd);

    return 0;
}

static void sync_time(uint32_t t)
{
    time_system_t *ts = s_time_system;

    ts->priv.timestamp = t;
    platform_set_utc_time(t, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_time_system_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, time_system_t);
    ptr->sync_time = sync_time;
    ptr->get_time = platform_get_utc_time;
    s_time_system = ptr;

    return ADD_MODULE("time_sys", ID_MODULE_TIME, 0, time_system_init, ptr,
         list_head, sizeof(time_system_t));
}

