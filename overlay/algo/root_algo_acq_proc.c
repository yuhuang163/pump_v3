/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "algorithm_manager.h"  // IWYU pragma: keep
#include "root_algo_acq_proc.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define ALGO_SAMPLING_DATA_POINT_NUM      10 /// 一包数据包含采集点个数

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t sampling_count = 0; /// 算法采集数据次数
static algo_sampling_data_t *algo_sampling_data = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void algo_sampling_data_cb(algo_manager_context_t *ctx, uint8_t *data, int data_len)
{
    int ret = 0;
    EXCEPTION_CHECK_VOID(ctx && data && ctx->priv.motor);
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)data;
    algo_sampling_data_t *sampling_data = &algo_sampling_data[sampling_count];
    motor_policy_info_t *info = ctx->priv.motor->priv.runtime.info;
    sampling_data->timestamp = get_timestamp_ms(0);
    /// 气压数据
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
    for (int i = 0; i < AIR_PRESSURE_NUM; i++) {
        sampling_data->air_pressure[i] = sensor_data->air_pressure_data[i].pressure;
    }
#endif
    /// 泵奶数据
    for (int i = 0; i < BREAST_PUMP_NUM; i++) {
        sampling_data->pump_state[i].state = info[i].state.status;
        sampling_data->pump_state[i].pump_duty = info[i].state.pump_duty;
        sampling_data->pump_state[i].valve_duty = info[i].state.valve_duty;
        sampling_data->pump_state[i].target_pressure = info[i].state.target_pressure;
        #if IS_SUPPORT_RELATIVE_RUN_TIME
            sampling_data->pump_state[i].run_time = info[i].state.run_time;
        #else
            sampling_data->pump_state[i].run_time = get_timestamp_ms(info[i].state.start_time);
        #endif
    }
    sampling_count++;
    if (sampling_count >= ALGO_SAMPLING_DATA_POINT_NUM) {
        servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
        EXCEPTION_CHECK_VOID(servers_ctx && servers_ctx->hook);
        servers_publish_data_args_t args = {
            .arg1 = sampling_count,
            .arg2 = 0,
            .val = algo_sampling_data,
        };
        servers_ctx->hook->factory_server_data_publish(SERVERS_PUBLISH_DATA_ALGO_SAMPLING, &args);
        sampling_count = 0;
    }

#if 0
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        LOG_PASS("%d pressure:%f raw:%d state:%d pump:%d valve:%d target:%d time:%d; ", 
            i, sensor_data->air_pressure_data[i].pressure, sensor_data->air_pressure_data[i].raw_data,
            info[i].state.status, 
            info[i].state.pump_duty, info[i].state.valve_duty,
            info[i].state.target_pressure, info[i].state.run_time);
    }
    LOG_PASS("\r\n");
#endif

}

void algo_sampling_start(algo_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    sampling_count = 0;
    ctx->priv.is_sampling = 1;
    algo_sampling_data = u_malloz(sizeof(algo_sampling_data_t) * ALGO_SAMPLING_DATA_POINT_NUM);
    LOG_ALGO("algo sampling start");
}

void algo_sampling_stop(algo_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    ctx->priv.is_sampling = 0;
    u_free(algo_sampling_data);
    algo_sampling_data = NULL;
    LOG_ALGO("algo sampling stop");
}

bool algo_sampling_is_sampling(algo_manager_context_t *ctx)
{
    EXCEPTION_CHECK_BOOL(ctx);
    return ctx->priv.is_sampling;
}

bool algo_change_sampling_interval(algo_manager_context_t *ctx, uint16_t interval)
{
    EXCEPTION_CHECK_BOOL(ctx);
    algo_sampling_stop(ctx);
    EXCEPTION_CHECK_BOOL(ctx->priv.sensor);
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
    air_pressure_context_t *air_pressure_ctx = (air_pressure_context_t *)ctx->priv.sensor->find_sensor(ID_SENSOR_AIR_PRESSURE);
    if (air_pressure_ctx) {
        air_pressure_ctx->set_sampling_interval(interval);
    }
#endif
    algo_sampling_start(ctx);
    return true;
}

