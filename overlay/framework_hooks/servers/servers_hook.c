/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_server_client.h"
#include "master_server_client.h"

/*
* DEFINES
*****************************************************************************************
*/
typedef struct {
    uint8_t force_update_flag;
    breast_pump_result_t pump_result;
    battery_state_info_t battery_state;
} servers_system_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static servers_system_info_t *servers_sys_info = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void on_breast_pump_result_cb(void *user_data, breast_pump_result_t *result)
{
    servers_manager_context_t *ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(ctx);

}

static void on_servers_battery_state_cb(void *user_data, battery_state_info_t *info)
{
    servers_manager_context_t *ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(ctx);
}

static int on_post_servers_init_route(struct servers_manager_context *ctx)
{
    factory_server_client_init();
    master_server_client_init();
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_INT(pump_ctx);
    ADD_BREAST_PUMP_RESULT_CB(pump_ctx, ctx, on_breast_pump_result_cb);
    battery_manager_context_t *battery_ctx = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    ADD_BATTERY_STATE_CB(battery, ctx, on_servers_battery_state_cb);
    servers_sys_info = u_malloz(sizeof(servers_system_info_t));
    return 0;
}

static int on_servers_manager_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    on_master_app_old_protocol_receiver_route(id, context, arg1, arg2, user_data);
    return 0;
}

static int servers_work_thread_route(servers_manager_context_t *ctx, void *msg)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_servers_manager_hook(servers_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __servers_manager_hook));
    ctx->hook->on_post_init_route = on_post_servers_init_route;
    ctx->hook->on_servers_manager_receiver_route = on_servers_manager_receiver_route;
    ctx->hook->factory_server_data_publish = factory_server_data_publish;
    ctx->hook->app_old_protocol_server_data_publish = app_old_protocol_server_data_publish;
    ctx->hook->servers_work_thread_route = servers_work_thread_route;
}
