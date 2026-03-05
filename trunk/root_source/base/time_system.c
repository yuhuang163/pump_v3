
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
static int time_system_init(void *ptr)
{
    time_system_t *ts = ptr;
    uint32_t rtc_timestamp = platform_get_utc_time();
    uint32_t timestamp = 0;
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (ctx && ctx->get_store_val) {
        ctx->get_store_val(STORE_ID_SYS_UTC_TIMESTAMP, 0, 0, &timestamp);
    }
    utc_time_t utc_time = {0};
    timestamp_to_utc_time(rtc_timestamp, &utc_time);
    LOGW("timestamp: %u, utc_time: %u-%d-%d %d:%d:%d", 
         timestamp, utc_time.year, utc_time.month, utc_time.day,
         utc_time.hour, utc_time.minute, utc_time.second);
    if (rtc_timestamp > PRODUCT_BUILD_TIMESTAMP && rtc_timestamp > timestamp) {
        ts->priv.last_time_sync_utc = timestamp; /// 上一次同步UTC时间
        return 0;
    }  
    ts->priv.last_time_sync_utc = 0;
    timestamp =  PRODUCT_BUILD_TIMESTAMP; 
    platform_set_utc_time(timestamp, 0);
    LOGW("timestamp reset");
    return 0;
}

static void sync_utc_time(uint32_t t)
{
    time_system_t *ts = s_time_system;
    ts->priv.last_time_sync_utc = t;
    platform_set_utc_time(t, 0);
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (ctx && ctx->put_store_val) {
        ctx->put_store_val(STORE_ID_SYS_UTC_TIMESTAMP, t, 0, NULL);
    }
}

static uint32_t get_last_time_sync_utc(void)
{
    time_system_t *ts = s_time_system;
    EXCEPTION_CHECK_UINT(ts);
    return ts->priv.last_time_sync_utc;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_time_system_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, time_system_t);
    ptr->sync_utc_time = sync_utc_time;
    ptr->get_utc_time = platform_get_utc_time;
    ptr->get_last_time_sync_utc = get_last_time_sync_utc;
    s_time_system = ptr;
    return ADD_MODULE("time_sys", ID_MODULE_TIME, 0, time_system_init, ptr,
         list_head, sizeof(time_system_t));
}

