/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep

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
static int on_post_ble_init_route(ble_manager_context_t *ctx)
{
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_ble_hook(ble_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __ble_manager_hook));
    ctx->hook->on_post_init_route = on_post_ble_init_route;
}

