/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#ifndef LOG_FILTER
#define LOG_FILTER (\
    FILTER_MOTOR|FILTER_UI|FILTER_KEY|FILTER_PM|FILTER_DB|FILTER_STORE| \
    FILTER_BCR|FILTER_WM|FILTER_AUDIO|FILTER_HG|FILTER_HDR|FILTER_PBUS|FILTER_OTA| \
    FILTER_RES|FILTER_BLE|FILTER_MISC|FILTER_STEP|FILTER_PROTOCOL|FILTER_AIR \
)
#endif

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
static void init_thread(const void *thd)
{
    platform_post_init();
    uint32_t t0 = get_timestamp();
    init_hang_detect_context();
    init_broadcast_context();
    init_handler_post_context();
    register_module();
    framework_extra_hook();
    init_module();
    init_user_log_filter();
    run_user_thread_dispatch();
    LOG_STEP("init escaped: %lu", get_timestamp_ms(t0));
    EXIT_THREAD((thread_context_t *)thd);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void root_main(int argc, const char *argv[])
{
    static thread_context_t init_thd = {0};
    platform_prepare_init();
    reset_log_filter(LOG_FILTER);
    RUN_NEW_THREAD(&init_thd, "init_thd", init_thread, RT_PRIO_MAIN_THREAD, (MAIN_THREAD_STACKSIZE));
    os_kernel_start_wrapper();
}
 