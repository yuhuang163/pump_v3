/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "algorithm_manager.h"  // IWYU pragma: keep
#include "root_algo_core.h" // IWYU pragma: keep
#include "root_algo_acq_proc.h" // IWYU pragma: keep

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
static void on_sensor_data_algo_cb(void *user_data, uint8_t *data, int data_len)
{
    algo_manager_context_t *ctx = user_data;
    EXCEPTION_CHECK_VOID(ctx);
    if (ctx->priv.is_sampling) {
        algo_sampling_data_cb(ctx, data, data_len);
    }
}

static void  hdr_broadcast_sys_boot_completed(hdr_post_args_t *args)
{
    algo_manager_context_t *ctx = args->user_data;
    EXCEPTION_CHECK_VOID(ctx);
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    sensor->add_sensor_evt_lisenser(ctx, AIR_PRESSURE_EVT_POLLING, 1, on_sensor_data_algo_cb);
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
                        // root_algo_reset(ctx);
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
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_algo_hook(algo_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __algo_manager_hook));
    ctx->hook->on_post_init_route = on_post_algo_init_route;
    ctx->hook->on_algo_receiver_route = on_algo_receiver_route;
#if IS_SUPPORT_ALGO_SAMPLING
    ctx->hook->algo_sampling_start = algo_sampling_start;
    ctx->hook->algo_sampling_stop = algo_sampling_stop;
#endif
}
