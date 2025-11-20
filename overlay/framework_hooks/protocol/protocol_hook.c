/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_protocol_parse.h"
#include "app_protocol_parse.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static int on_post_protocol_manager_init_route(struct protocol_manager_context *ctx)
{
    on_post_factory_protocol_init(ctx);
    on_post_app_commication_protocol_init(ctx);
    return 0;
}

static int on_perpare_protocol_manager_init_route(struct protocol_manager_context *ctx)
{
    on_perpare_factory_protocol_init(ctx);
    on_perpare_app_commication_protocol_init(ctx);
    return 0;
}

static int on_protocol_manager_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    on_factory_protocol_br_receiver_route(id, context, arg1, arg2, user_data);
    on_app_commication_protocol_br_receiver_route(id, context, arg1, arg2, user_data);
    return 0;
}
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_protocol_manager_hook(protocol_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct protocol_manager_hook));
    ctx->hook->on_perpare_init_route = on_post_protocol_manager_init_route;
    ctx->hook->on_post_init_route = on_perpare_protocol_manager_init_route;
    ctx->hook->on_protocol_manager_receiver_route = on_protocol_manager_br_receiver_route;
    ctx->hook->factory_tests_protocol_parse_route = product_tests_protocol_parse_route;
    ctx->hook->app_commication_protocol_parse_route = app_commication_protocol_parse_route;

}
