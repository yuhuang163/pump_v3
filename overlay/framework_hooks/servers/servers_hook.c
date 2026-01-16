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

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static int on_post_servers_init_route(struct servers_manager_context *ctx)
{
    factory_server_client_init();
    LOG_SRV("factory_server_client_init");
    master_server_client_init();
    LOG_SRV("master_server_client_init");
    return 0;
}

static int on_servers_manager_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    on_master_app_old_protocol_receiver_route(id, context, arg1, arg2, user_data);
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
}
