/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "algorithm_manager.h"  // IWYU pragma: keep
#include "root_algo_core.h" // IWYU pragma: keep
#include "algo_fusion_lib.h" // IWYU pragma: keep

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
void algo_fusion_data_cb(algo_manager_context_t *ctx, uint8_t *data, int data_len)
{
    int ret = 0;
    EXCEPTION_CHECK_VOID(ctx && data && ctx->priv.motor);
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)data;
    algo_air_pressure_data_t algo_data = {0};
    motor_policy_info_t *info = ctx->priv.motor->priv.runtime.info;
    pump_setting_t current_setting = {0};
    int time_index = 0;
    READ_PUMP_MODE_STEP_INDEX(ctx->priv.pump, time_index);
    READ_PUMP_SETTING_INFO(ctx->priv.pump, time_index, current_setting);
    uint8_t time_policy_mode =  ctx->priv.motor->get_runtime_mode_type(0);

    algo_data.timestamp = get_timestamp_ms(0);
    /// 气压数据
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
    for (int i = 0; i < ALGO_AIR_PRESSURE_NUM; i++) {
        if (i >= AIR_PRESSURE_NUM) {
            break;
        }
        algo_data.air_pressure[i] = sensor_data->air_pressure_data[i].pressure;
    }
#endif
    /// 泵奶数据
    //algo_data.do_work = ctx->priv.motor->is_on_motor_task_running(ctx->priv.motor); 
    {
        int is_working = 0;
        READ_PUMP_IS_WORKING(ctx->priv.pump, is_working);
        algo_data.do_work = (uint8_t)is_working;
    }

    for (int i = 0; i < ALGO_AIR_PRESSURE_NUM; i++) {
        if (i >= BREAST_PUMP_NUM) {
            break;
        }
        algo_data.pump_state[i].level = current_setting.level[i];
        algo_data.pump_state[i].mode = time_policy_mode;
        algo_data.pump_state[i].state = info[i].state.status;
        algo_data.pump_state[i].pump_duty = info[i].state.pump_duty;
        algo_data.pump_state[i].valve_duty = info[i].state.valve_duty;
        algo_data.pump_state[i].target_pressure = info[i].state.target_pressure;
        algo_data.pump_state[i].target_pump_time = info[i].original.pumping_time;
        #if IS_SUPPORT_RELATIVE_RUN_TIME
            algo_data.pump_state[i].run_time = 0;
        #else
            algo_data.pump_state[i].run_time = get_timestamp_ms(info[i].state.start_time);
        #endif
    }

#if 0
    static uint16_t log_time = 0;
    log_time++;
    if (log_time % 100 == 0) {
        LOG_PASS("algo data level:%d/%d target:%d/%d, mode:%d/%d, duty:%d/%d", 
            algo_data.pump_state[0].level, algo_data.pump_state[1].level, 
            algo_data.pump_state[0].target_pressure, algo_data.pump_state[1].target_pressure,
            algo_data.pump_state[0].mode, algo_data.pump_state[1].mode,
            algo_data.pump_state[0].pump_duty, algo_data.pump_state[1].pump_duty);
    }
#endif

#if IS_SUPPORT_ALGO_LIB_LOG_OUTPUT
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        LOG_PASS("%d level:%d mode:%d state:%d pump:%d valve:%d target:%d time:%d pressure:%f", 
            i, current_setting.level[i], time_policy_mode, info[i].state.status, 
            info[i].state.pump_duty, info[i].state.valve_duty,
            info[i].state.target_pressure, algo_data.pump_state[i].run_time, algo_data.air_pressure[i]);
    }
    LOG_PASS("\r\n");
#endif
    algo_fusion_input_data_t input = {
        .data_type = ALGO_SENSOR_ATTR_AIR_PRESSURE,
        .sensor_data = &algo_data,
    };
    algo_fusion_lib_input(&input);
}

