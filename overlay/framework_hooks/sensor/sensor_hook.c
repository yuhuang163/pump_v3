/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "sensor_manager.h"  // IWYU pragma: keep
#if IS_SUPPORT_DEVICE_IMU
#include "imu_module_internal.h"
#endif
#if IS_SUPPORT_DEVICE_FSENSOR
#include "force_sensor_module_internal.h"
#endif
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
#include "air_pressure_module_internal.h"
#endif
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR
#include "light_sensor_module_internal.h"
#endif

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
/// sensor事件分发.
static int dispatch_sensor_client(sensor_manager_context_t *obj, sensor_wrapper_data_t *sensor_data)
{
    list_head_t *head = &obj->priv.client_list;
    
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        sensor_evt_client_node_t *node = CONTAINER_OF(pos, sensor_evt_client_node_t, list);
        if (node->report_type == IMU_EVT_POLLING ) {
            #if IS_SUPPORT_DEVICE_IMU
            dispatch_imu_client(sensor_data, node);
            #endif
        } else if (node->report_type == F_SENSOR_EVT_POLLING) {
            #if IS_SUPPORT_DEVICE_FSENSOR
            dispatch_force_sensor_client(sensor_data, node);
            #endif
        } else if (node->report_type == AIR_PRESSURE_EVT_POLLING) {
            #if IS_SUPPORT_DEVICE_AIR_PRESSURE
            dispatch_air_pressure_client(sensor_data, node);
            #endif
        } else if (node->report_type == LIGHT_SENSOR_EVT_POLLING) {
            #if IS_SUPPORT_DEVICE_LIGHT_SENSOR
            dispatch_light_sensor_client(sensor_data, node);
            #endif
        }
    }
    MUTEX_UNLOCK(obj->priv.m_lock);

    return 0;
}

static int on_sensor_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    #if IS_SUPPORT_DEVICE_IMU
    on_imu_sensor_receiver(id, context, arg1, arg2, user_data);
    #endif

    return 0;
}

int sensor_thread_route(struct sensor_manager_context *ctx, sensor_wrapper_data_t *sensor_data)
{
    #if IS_SUPPORT_DEVICE_IMU
    imu_polling_handler(sensor_data);
    #endif
    #if IS_SUPPORT_DEVICE_FSENSOR
    force_sensor_polling_handler(sensor_data);
    #endif
    #if IS_SUPPORT_DEVICE_AIR_PRESSURE
    air_pressure_polling_handler(sensor_data);
    #endif
    #if IS_SUPPORT_DEVICE_LIGHT_SENSOR
    light_sensor_polling_handler(sensor_data);
    #endif
    dispatch_sensor_client(ctx, sensor_data);
    return 0;
}

static int on_post_sensor_init_route(struct sensor_manager_context *ctx)
{
    #if IS_SUPPORT_DEVICE_IMU
    on_post_imu_init(ctx);
    #endif
    return 0;
}

static int on_perpare_sensor_init_route(struct sensor_manager_context *ctx)
{
    #if IS_SUPPORT_DEVICE_IMU
    on_perpare_imu_init(ctx);
    #endif
    #if IS_SUPPORT_DEVICE_FSENSOR
    on_perpare_force_sensor_init(ctx);
    #endif
    #if IS_SUPPORT_DEVICE_AIR_PRESSURE
    on_perpare_air_pressure_init(ctx);
    #endif
    #if IS_SUPPORT_DEVICE_LIGHT_SENSOR
    on_perpare_light_sensor_init(ctx);
    #endif
    return 0;
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_sensor_hook(sensor_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __sensor_manager_hook));
    ctx->hook->on_perpare_init_route = on_perpare_sensor_init_route;
    ctx->hook->on_post_init_route = on_post_sensor_init_route;
    ctx->hook->on_sensor_receiver_route = on_sensor_receiver_route;
    ctx->hook->sensor_thread_route = sensor_thread_route;
    #if IS_SUPPORT_DEVICE_IMU
    register_imu_module_ctx(ctx);
    #endif
}
