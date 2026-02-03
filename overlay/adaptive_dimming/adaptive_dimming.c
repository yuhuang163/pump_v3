/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_light_sensor.h"
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "sensor_manager_api.h"
#include <stdint.h>
#include "adaptive_dimming.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define ADAP_DIM_INVALID_DUTY            255            /// 无效的LED亮度等级

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t last_lux_for_duty = 0;           /// 上一次计算duty时的lux值
static adaptive_dimming_info_t *adaptive = NULL;
static uint16_t light_sensor_data[10] = {0};
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void on_light_sensor_data_cb(void *user_data, uint8_t *data, int data_len)
{
    EXCEPTION_CHECK_VOID(adaptive);
    EXCEPTION_CHECK_VOID(user_data);
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)data;
    if ( adaptive->is_sampling) {
        //上报
        light_sensor_data_report(data);
        return;
    }
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR
    if (adaptive->capture_data.is_acquire_complete == 0 && 
        adaptive->capture_data.index < ADAP_DIM_DATA_SIZE) {
        adaptive->capture_data.buff[adaptive->capture_data.index] = sensor_data->light_sensor_data.lux_level;
        adaptive->capture_data.index++;
        if (adaptive->capture_data.index >= ADAP_DIM_DATA_SIZE) {
            adaptive->capture_data.is_acquire_complete = 1;
            adaptive->capture_data.index = 0;
            // LOG_MISC("capture data complete");
        }
    }
#endif
}

static int adaptive_dimming_duty_calculate(uint16_t lux_avg, uint8_t *out_duty)
{
    uint8_t new_duty = 0;
    /// 限制 lux 值在有效范围内
    if (lux_avg > ADAP_DIM_LUX_MAX) {
        lux_avg = ADAP_DIM_LUX_MAX;
    }
    
    /// 计算滞后阈值，避免在临界值附近来回切换
    const uint16_t hysteresis_threshold = (lux_avg > ADAP_DIM_HYST_THR_BASE) ? 
                                         (lux_avg / ADAP_DIM_HYST_THR_RATIO) : 
                                         ADAP_DIM_HYST_THR_MIN;
    
    /// 根据 lux 值分段计算 duty（环境越亮，LED 越暗）
    if (lux_avg < ADAP_DIM_LUX_THR_VDARK) {
        new_duty = ADAP_DIM_LED_LVL_VDARK;
    } else if (lux_avg < ADAP_DIM_LUX_THR_DARK) {
        /// 线性映射: lux 10->100, lux 50->80
        new_duty = ADAP_DIM_LED_LVL_VDARK - 
                   (lux_avg - ADAP_DIM_LUX_THR_VDARK) * 
                   (ADAP_DIM_LED_LVL_VDARK - ADAP_DIM_LED_LVL_DARK_MIN) / 
                   (ADAP_DIM_LUX_THR_DARK - ADAP_DIM_LUX_THR_VDARK);
        if (new_duty < ADAP_DIM_LED_LVL_DARK_MIN) {
            new_duty = ADAP_DIM_LED_LVL_DARK_MIN;
        }
    } else if (lux_avg < ADAP_DIM_LUX_THR_MEDIUM) {
        /// 线性映射: lux 50->80, lux 200->50
        new_duty = ADAP_DIM_LED_LVL_DARK_MIN - 
                   (lux_avg - ADAP_DIM_LUX_THR_DARK) * 
                   (ADAP_DIM_LED_LVL_DARK_MIN - ADAP_DIM_LED_LVL_MEDIUM_MIN) / 
                   (ADAP_DIM_LUX_THR_MEDIUM - ADAP_DIM_LUX_THR_DARK);
        if (new_duty < ADAP_DIM_LED_LVL_MEDIUM_MIN) {
            new_duty = ADAP_DIM_LED_LVL_MEDIUM_MIN;
        }
    } else if (lux_avg < ADAP_DIM_LUX_THR_BRIGHT) {
        /// 线性映射: lux 200->50, lux 500->20
        new_duty = ADAP_DIM_LED_LVL_MEDIUM_MIN - 
                   (lux_avg - ADAP_DIM_LUX_THR_MEDIUM) * 
                   (ADAP_DIM_LED_LVL_MEDIUM_MIN - ADAP_DIM_LED_LVL_BRIGHT_MIN) / 
                   (ADAP_DIM_LUX_THR_BRIGHT - ADAP_DIM_LUX_THR_MEDIUM);
        if (new_duty < ADAP_DIM_LED_LVL_BRIGHT_MIN) {
            new_duty = ADAP_DIM_LED_LVL_BRIGHT_MIN;
        }
    } else if (lux_avg < ADAP_DIM_LUX_THR_VBRIGHT) {
        new_duty = ADAP_DIM_LED_LVL_VBRIGHT;
    } else {
        new_duty = ADAP_DIM_LED_LVL_OFF;
    }
    
    /// 滞后机制：只有当 lux 值变化超过阈值时才改变亮度，避免闪烁
    if (last_lux_for_duty == 0) {
        last_lux_for_duty = lux_avg;
        *out_duty = new_duty;
        LOG_MISC("lux_avg: %d, new_duty: %d", lux_avg, new_duty);
        return 1;
    } 
    uint16_t lux_diff = (lux_avg > last_lux_for_duty) ? \
            (lux_avg - last_lux_for_duty) : (last_lux_for_duty - lux_avg);
    if (lux_diff >= hysteresis_threshold) {
        LOG_MISC("lux_diff: %d, hysteresis_threshold: %d, new_duty: %d, lux_avg: %d",
                lux_diff, hysteresis_threshold, new_duty, lux_avg);     
        last_lux_for_duty = lux_avg;
        *out_duty = new_duty;
        return 1;
    }
    return 0;
}

static void adaptive_dimming_polling_thread(void *user_data)
{
    led_manager_context_t *led = (led_manager_context_t *)user_data;
    EXCEPTION_CHECK_VOID(led);
    EXCEPTION_CHECK_VOID(adaptive);
    if (adaptive->capture_data.is_acquire_complete == 0) {
        return;
    }
    uint32_t lux_sum = 0, lux_avg = 0;
    for (int i = 0; i < ADAP_DIM_DATA_SIZE; i++) {
        lux_sum += adaptive->capture_data.buff[i];
    }
    adaptive->capture_data.is_acquire_complete = 0; /// 采集完成标志清零
    lux_avg = lux_sum / ADAP_DIM_DATA_SIZE;
    uint8_t refresh_duty = 0;
    if (adaptive_dimming_duty_calculate(lux_avg, &refresh_duty) == 0) {
        return;
    }

    if ( led->hook && led->hook->night_light_adaptive_level_update) {
        led->hook->night_light_adaptive_level_update(led, refresh_duty);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void adaptive_dimming_init(void)
{
    adaptive = (adaptive_dimming_info_t *)u_malloz(sizeof(adaptive_dimming_info_t));
    EXCEPTION_CHECK_VOID(adaptive);
    adaptive->is_sampling = 0;
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_VOID(misc);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led);
    misc->add_misc_task(led, MISC_TASK_ID_ADAPTIVE_DIMMING, 200, MISC_WORK_MSG_ID_TIMER, adaptive_dimming_polling_thread);
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    EXCEPTION_CHECK_VOID(sensor);
    sensor->add_sensor_evt_lisenser(sensor, LIGHT_SENSOR_EVT_POLLING, 1, on_light_sensor_data_cb);
}

void light_sensor_sampling_start(void)
{
    EXCEPTION_CHECK_VOID(adaptive);
    adaptive->is_sampling = 1;
    LOG_MISC("light sensor sampling start");
}

void light_sensor_sampling_stop(void)
{
    EXCEPTION_CHECK_VOID(adaptive);
    adaptive->is_sampling = 0;
    LOG_MISC("light sensor sampling stop");
}

bool light_sensor_sampling_is_sampling(void)
{
    EXCEPTION_CHECK_BOOL(adaptive);
    return adaptive->is_sampling;
}

bool light_sensor_change_sampling_interval(uint16_t interval)
{
    EXCEPTION_CHECK_BOOL(adaptive);
    if (interval < 10) {
        LOG_MISC("sampling interval error");
        return 0;
    }
    light_sensor_sampling_stop();
    /// 设置采样间隔
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    EXCEPTION_CHECK_BOOL(sensor);
    light_sensor_context_t *light_sensor = (light_sensor_context_t *)sensor->find_sensor(ID_SENSOR_LIGHT_SENSOR);
    EXCEPTION_CHECK_BOOL(light_sensor);
    light_sensor->set_sampling_interval(interval);
    light_sensor_sampling_start();
    return true;
}

bool light_sensor_data_report(uint8_t *data)
{
    sensor_wrapper_data_t *sensor_data = (sensor_wrapper_data_t *)data;
    static uint8_t light_sample_count = 0;
    if (light_sample_count < 10) {
        light_sensor_data[light_sample_count] = sensor_data->light_sensor_data.lux_level;
    }
    light_sample_count++;
    if (light_sample_count >= 10) {
        servers_manager_context_t *servers_ctx =
            (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
        EXCEPTION_CHECK_BOOL(servers_ctx && servers_ctx->hook);
        servers_publish_data_args_t args = {
            .arg1 = 10,
            .arg2 = 0,
            .val = light_sensor_data,
        };
        servers_ctx->hook->factory_server_data_publish(
            SERVERS_PUBLISH_DATA_LIGHT_SENSOR_SAMPLING, &args);
        light_sample_count = 0;
    }
}
