/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "algorithm_manager.h"  // IWYU pragma: keep
#include "root_algo_core.h" // IWYU pragma: keep
#if IS_SUPPORT_ADAPTIVE_DIMMING
#include "adaptive_dimming.h" // IWYU pragma: keep
#endif

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static algo_manager_context_t *algo_ctx = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_ADAPTIVE_DIMMING

#endif

static void hdr_broadcast_sys_boot_completed(hdr_post_args_t *args)
{
    algo_manager_context_t *ctx = args->user_data;
    EXCEPTION_CHECK_VOID(ctx);
    root_algo_init(ctx);
    root_algo_sensor_cb_register(ctx);
    ctx->priv.algo_inited = 1;
    LOG_ALGO("algo_inited ok, get_version: %d", ctx->get_algo_version());
}

static int on_algo_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    algo_manager_context_t *ctx = context;
    switch (id.code) {
        case ID_BROADCAST_SYS_BOOT_COMPLETED:
            {
                LOG_ALGO("algo_inited");
                POST_TASK_HANDLER_DELAYED_ARG1(hdr_broadcast_sys_boot_completed, 0, ctx, "algo_fusion");
            }
            break;
        case ID_BROADCAST_PM_STATE_CHANGE: 
            {
                if (arg1 == ID_SYS_PM_SUSPEND) {
                    LOG_ALGO("algo cannot use");
                } else {
                    if (ctx->priv.algo_inited == 1) {
                        root_algo_reset(ctx);
                    }
                }
            }
            break;
        default:
            break;
    }
    return 0;
}

static int on_post_algo_init_route(algo_manager_context_t *ctx)
{
    algo_ctx = ctx;
#if IS_SUPPORT_ADAPTIVE_DIMMING
    adaptive_dimming_init();
#endif
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_algo_hook(algo_manager_context_t *ctx)
{
    ctx->get_algo_version = root_algo_version_get;
    ctx->hook = u_malloz(sizeof(struct __algo_manager_hook));
    ctx->hook->on_post_init_route = on_post_algo_init_route;
    ctx->hook->on_algo_receiver_route = on_algo_receiver_route;
#if IS_SUPPORT_ALGO_SAMPLING
    ctx->hook->algo_sampling_start = algo_sampling_start;
    ctx->hook->algo_sampling_stop = algo_sampling_stop;
    ctx->hook->algo_sampling_is_sampling = algo_sampling_is_sampling;
    ctx->hook->algo_change_sampling_interval = algo_change_sampling_interval;
#endif
#if IS_SUPPORT_ADAPTIVE_DIMMING
    ctx->hook->set_adaptive_dimming_state = set_adaptive_dimming_state;
    ctx->hook->get_adaptive_dimming_state = get_adaptive_dimming_state;
#endif
}
