/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
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
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR
    if (adaptive->capture_data.is_acquire_complete == 0 && 
        adaptive->capture_data.index < ADAP_DIM_DATA_SIZE) {
        adaptive->capture_data.buff[adaptive->capture_data.index] = sensor_data->light_sensor_data.lux_level;
        adaptive->capture_data.index++;
        if (adaptive->capture_data.index >= ADAP_DIM_DATA_SIZE) {
            adaptive->capture_data.is_acquire_complete = 1;
            adaptive->capture_data.index = 0;
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
        LOG_ALGO("lux_avg: %d, new_duty: %d", lux_avg, new_duty);
        return 1;
    } 
    uint16_t lux_diff = (lux_avg > last_lux_for_duty) ? \
            (lux_avg - last_lux_for_duty) : (last_lux_for_duty - lux_avg);
    if (lux_diff >= hysteresis_threshold) {
        LOG_ALGO("lux_diff: %d, hysteresis_threshold: %d, new_duty: %d",
                lux_diff, hysteresis_threshold, new_duty);     
        last_lux_for_duty = lux_avg;
        *out_duty = new_duty;
        return 1;
    }
    return 0;
}

static void adaptive_dimming_polling_thread(void *user_data)
{
    rt_device_node_t *night_light = (rt_device_node_t *)user_data;
    EXCEPTION_CHECK_VOID(night_light);
    EXCEPTION_CHECK_VOID(adaptive);
    if (!adaptive->is_adaptive_dimming) {
        return;
    }

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
    night_light_pwm_duty_set_msg_t msg = {
        .led_index = NIGHT_LIGHT_LED_ID_0,
        .pwm_duty = refresh_duty,
    };
    night_light->ioctl(night_light, OP_CODE_NIGHT_LIGHT_PWM_DUTY_SET, (void *)&msg);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void set_adaptive_dimming_state(bool state)
{
    EXCEPTION_CHECK_VOID(adaptive);
    adaptive->is_adaptive_dimming = state;
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    night_light_info_t light_info = {0};
    light_info.adaptive = state;
    st->put_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_INFO, 0, 0, &light_info);
    last_lux_for_duty = 0;
}

bool get_adaptive_dimming_state(void)
{
    EXCEPTION_CHECK_BOOL(adaptive);
    return adaptive->is_adaptive_dimming;
}

void adaptive_dimming_init(void)
{
    adaptive = (adaptive_dimming_info_t *)u_malloz(sizeof(adaptive_dimming_info_t));
    EXCEPTION_CHECK_VOID(adaptive);
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_VOID(misc);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    sensor_manager_context_t *sensor = (sensor_manager_context_t *)find_module(ID_MODULE_SENSOR);
    EXCEPTION_CHECK_VOID(sensor);
    rt_device_node_t *night_light = pbus->find_device(ID_DEVICE_LED);
    night_light_info_t light_info = {0};
    sensor->add_sensor_evt_lisenser(sensor, LIGHT_SENSOR_EVT_POLLING, 1, on_light_sensor_data_cb);
    st->get_store_ram_val(STORE_ID_SYS_NIGHT_LIGHT_INFO, 0, 0, &light_info);
    adaptive->is_adaptive_dimming = light_info.adaptive;
    misc->add_misc_task(night_light, MISC_TASK_ID_ADAPTIVE_DIMMING, 500,
         MISC_WORK_MSG_ID_TIMER, adaptive_dimming_polling_thread);
    LOG_ALGO("adaptive_dimming_state_sync: %d", adaptive->is_adaptive_dimming);
}