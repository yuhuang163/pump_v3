/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_timer.h"
#include "bsp_common.h"  // IWYU pragma: keep
#include "bsp_config.h"  // IWYU pragma: keep
#include <string.h>

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_STALL_DETECTION_LOG      (1)
#define IS_SUPPORT_STALL_DETECTION_ADC_VALUE_DEBUG (0)
#define STALL_DETECTION_INTERVAL_MS         (5)  /// 堵转检测间隔时间，单位ms
#define ADC_SAMPLING_DELAY_MS               (5)  /// 采样延迟时间，单位ms
#define MAX_STALL_DETECTION_TARGET_TIMES    (200)

typedef struct {
    uint8_t  is_running;        /// 运行状态，0：停止，1：运行
    uint16_t target_times;      /// 目标次数
    uint16_t work_times;        /// 工作次数
    uint16_t last_voltage;      /// 最后一次电压
    uint16_t pulse_width;       /// 脉冲宽度
    uint32_t sum_voltage;       /// 总电压
} motor_pump_stall_detection_t;

typedef struct {
    HWTIMER_ID time_id;               /// 定时器ID
    uint8_t pump_id;                  /// 泵ID
    uint8_t is_running;               /// 定时器运行状态，0：停止，1：运行
} motor_pump_stall_detection_timer_t;

typedef struct {
    uint16_t pulse_width;           /// 脉冲宽度
    uint16_t threshold_max;         /// 阈值最大值
    uint16_t threshold_min;         /// 阈值最小值
} stall_detection_threshold_t;

typedef struct {
    uint16_t adc_value_max;           /// ADC值最大值
    uint16_t adc_value_min;           /// ADC值最小值
    uint16_t adc_value_avg;           /// ADC值平均值
    uint16_t adc_value_count;         /// ADC值采集计数
    uint16_t *adc_value_all;          /// ADC值数组指针
} stall_detection_adc_value_t;
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static motor_pump_stall_detection_t s_pump_stall_status[MOTOR_PWM_ID_MAX] = {0};
static motor_pump_stall_detection_timer_t s_pump_stall_timer[MOTOR_PWM_ID_MAX] = {0};
const static stall_detection_threshold_t s_pump_stall_threshold[100] = {0};
static stall_detection_adc_value_t s_pump_stall_adc_value[MOTOR_PWM_ID_MAX] = {0};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void bsp_pump_stall_detection_timer_start(motor_pump_id_e id, uint32_t period)
{
    int ret = 0;
    if (id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump_id is out range: %d", id);
        return;
    }

    if (s_pump_stall_timer[id].time_id == NULL) {
        LOGE("pump_stall_timer[%d].time_id is NULL", id);
        return;
    }

    if (hwtimer_active(s_pump_stall_timer[id].time_id)) {
        return;
    }
    
    ret = hwtimer_start(s_pump_stall_timer[id].time_id, MS_TO_HWTICKS(period));
    if (ret != E_HWTIMER_OK) {
        LOGE("time start failed, ret=%d, time_id=%p", ret, s_pump_stall_timer[id].time_id);
        return;
    }
    s_pump_stall_timer[id].is_running = true;
}

static void bsp_pump_stall_detection_timer_stop(motor_pump_id_e id)
{
    if (id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump_id is out range: %d", id);
        return;
    }

    if (s_pump_stall_timer[id].time_id == NULL) {
        LOGE("pump_stall_timer[%d].time_id is NULL", id);
        return;
    }

    if (hwtimer_active(s_pump_stall_timer[id].time_id)) {
        int ret = hwtimer_stop(s_pump_stall_timer[id].time_id);
        if (ret != E_HWTIMER_OK && ret != E_HWTIMER_IN_CALLBACK) {
            LOGE("time stop failed, ret=%d", ret);
            return;
        }
    }
    s_pump_stall_timer[id].is_running = false;
}

static void motor1_adc_irq_cb(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    s_pump_stall_status[0].work_times++;
    s_pump_stall_status[0].sum_voltage += volt;
    s_pump_stall_status[0].is_running = false;
#if IS_SUPPORT_STALL_DETECTION_ADC_VALUE_DEBUG
    s_pump_stall_adc_value[0].adc_value_all[s_pump_stall_adc_value[0].adc_value_count] = volt;
    s_pump_stall_adc_value[0].adc_value_count++;
    if (volt > s_pump_stall_adc_value[0].adc_value_max) {
        s_pump_stall_adc_value[0].adc_value_max = volt;
    }
    if (volt < s_pump_stall_adc_value[0].adc_value_min) {
        s_pump_stall_adc_value[0].adc_value_min = volt;
    }
#endif
    if (s_pump_stall_status[0].work_times >= s_pump_stall_status[0].target_times) {
        s_pump_stall_status[0].last_voltage = s_pump_stall_status[0].work_times > 0 ? \
            s_pump_stall_status[0].sum_voltage / s_pump_stall_status[0].work_times : 0;
    } else {
        bsp_pump_stall_detection_timer_start(MOTOR_PUMP_1, STALL_DETECTION_INTERVAL_MS);
    }
}

static void motor2_adc_irq_cb(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    s_pump_stall_status[1].work_times++;
    s_pump_stall_status[1].sum_voltage += volt;
    s_pump_stall_status[1].is_running = false;
#if IS_SUPPORT_STALL_DETECTION_ADC_VALUE_DEBUG
    s_pump_stall_adc_value[1].adc_value_all[s_pump_stall_adc_value[1].adc_value_count] = volt;
    s_pump_stall_adc_value[1].adc_value_count++;
    if (volt > s_pump_stall_adc_value[1].adc_value_max) {
        s_pump_stall_adc_value[1].adc_value_max = volt;
    }
    if (volt < s_pump_stall_adc_value[1].adc_value_min) {
        s_pump_stall_adc_value[1].adc_value_min = volt;
    }
#endif
    if (s_pump_stall_status[1].work_times >= s_pump_stall_status[1].target_times) {
        s_pump_stall_status[1].last_voltage = s_pump_stall_status[1].work_times > 0 ? \
            s_pump_stall_status[1].sum_voltage / s_pump_stall_status[1].work_times : 0;
    } else {
        bsp_pump_stall_detection_timer_start(MOTOR_PUMP_2, STALL_DETECTION_INTERVAL_MS);
    }
}

static int motor_pump_stall_detection_start(motor_pump_id_e pump_id, uint16_t target_times)
{
    int ret = 0;
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump id is out of range: %d", pump_id);
        return BSP_ERROR;
    }

    uint8_t adc_channel_id = (pump_id == MOTOR_PUMP_1) ? PUMP1_ADC_CHANNEL : PUMP2_ADC_CHANNEL;
    if (s_pump_stall_status[pump_id].is_running) {
        ret = hal_gpadc_close(adc_channel_id);
        if (ret != BSP_OK) {
            LOGE("adc channel: %d close failed", adc_channel_id);
        }
    }
    s_pump_stall_status[pump_id].work_times = 0;
    s_pump_stall_status[pump_id].sum_voltage = 0;
    s_pump_stall_status[pump_id].target_times = target_times;
    s_pump_stall_status[pump_id].last_voltage = 0;
    ret = hal_gpadc_open(adc_channel_id, HAL_GPADC_ATP_ONESHOT, pump_id == MOTOR_PUMP_1 ? motor1_adc_irq_cb : motor2_adc_irq_cb);
    if (ret != BSP_OK) {
        LOGE("adc channel: %d open failed", adc_channel_id);
    }
    s_pump_stall_status[pump_id].is_running = true;
    return ret;
}

static void motor_pump_stall_detection_continue(motor_pump_id_e pump_id)
{
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump id is out of range: %d", pump_id);
        return;
    }
    uint8_t adc_channel_id = (pump_id == MOTOR_PUMP_1) ? PUMP1_ADC_CHANNEL : PUMP2_ADC_CHANNEL;
    int ret = hal_gpadc_open(adc_channel_id, HAL_GPADC_ATP_ONESHOT, 
        pump_id == MOTOR_PUMP_1 ? motor1_adc_irq_cb : motor2_adc_irq_cb);
    if (ret != BSP_OK) {
        LOGE("adc channel: %d open failed", adc_channel_id);
    }
    s_pump_stall_status[pump_id].is_running = true;
}

static int motor_pump_stall_detection_stop(motor_pump_id_e pump_id)
{
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump id is out of range: %d", pump_id);
        return BSP_ERROR;
    }

    uint8_t adc_channel_id = (pump_id == MOTOR_PUMP_1) ? PUMP1_ADC_CHANNEL : PUMP2_ADC_CHANNEL;
    if (s_pump_stall_status[pump_id].is_running) {
        int ret = hal_gpadc_close(adc_channel_id);
        if (ret != BSP_OK) {
            LOGE("adc channel: %d close failed", adc_channel_id);
            return BSP_ERROR;
        }
    }
    s_pump_stall_status[pump_id].is_running = false;
    return BSP_OK;
}

static void bsp_pump_stall_detection_timer_handler(void *param)
{
    motor_pump_stall_detection_timer_t *timer_status = (motor_pump_stall_detection_timer_t *)param;
    timer_status->is_running = false;
    motor_pump_stall_detection_continue(timer_status->pump_id);
}

static void bsp_pump_stall_detection_timer_init(motor_pump_id_e pump_id)
{
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump id is out of range: %d", pump_id);
        return;
    }

    if (s_pump_stall_timer[pump_id].time_id) {
        return;
    }
    s_pump_stall_timer[pump_id].time_id = hwtimer_alloc(bsp_pump_stall_detection_timer_handler, &s_pump_stall_timer[pump_id]);
    s_pump_stall_timer[pump_id].is_running = false;
    s_pump_stall_timer[pump_id].pump_id = pump_id;
    LOG_PBUS("pump_stall_timer[%d].time_id=%p", pump_id, s_pump_stall_timer[pump_id].time_id);
}

static uint16_t bsp_pump_stall_detection_threshold_calculate(uint16_t pulse_width)
{
    /// todo:根据脉冲宽度计算阈值, 建议二分法，线性插值
    return 10000;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int pump_motor_stall_detection_start(uint8_t pump_id, uint16_t duration_ms, uint16_t pulse_width)
{
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump id is out of range: %d", pump_id);
        return BSP_ERROR;
    }
    uint16_t target_times = duration_ms / (STALL_DETECTION_INTERVAL_MS + ADC_SAMPLING_DELAY_MS);
    // PRINT_LOG("pump %d target_times: %d\r\n", pump_id, target_times);
    if (target_times > MAX_STALL_DETECTION_TARGET_TIMES) {
        target_times = MAX_STALL_DETECTION_TARGET_TIMES;
    }

#if IS_SUPPORT_STALL_DETECTION_ADC_VALUE_DEBUG
    s_pump_stall_adc_value[pump_id].adc_value_all = (uint16_t *)u_malloz(sizeof(uint16_t) * target_times);
    if (s_pump_stall_adc_value[pump_id].adc_value_all == NULL) {
        LOGE("malloc adc_value_all failed");
        return BSP_ERROR;
    }
    s_pump_stall_adc_value[pump_id].adc_value_count = 0;
    s_pump_stall_adc_value[pump_id].adc_value_max = 0;
    s_pump_stall_adc_value[pump_id].adc_value_min = 0XFFFF;
    s_pump_stall_adc_value[pump_id].adc_value_avg = 0;
#endif

    s_pump_stall_status[pump_id].pulse_width = pulse_width;
    bsp_pump_stall_detection_timer_init(pump_id);
    return motor_pump_stall_detection_start(pump_id, target_times);
}

int pump_motor_stall_detection_stop(uint8_t pump_id)
{
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump id is out of range: %d", pump_id);
        return BSP_ERROR;
    }
    bsp_pump_stall_detection_timer_stop(pump_id);
    /// 必须先关闭 GPADC，再 u_free(adc_value_all)；否则 motor1/2_adc_irq_cb 仍可能写已释放内存
    int ret = motor_pump_stall_detection_stop(pump_id);

#if IS_SUPPORT_STALL_DETECTION_ADC_VALUE_DEBUG
    if (ret == BSP_OK && s_pump_stall_adc_value[pump_id].adc_value_all) {
        LOG_PASS("pump %d adc_value_all: ", pump_id);
        for (int i = 0; i < s_pump_stall_adc_value[pump_id].adc_value_count; i++) {
            LOG_PASS("%d ", s_pump_stall_adc_value[pump_id].adc_value_all[i]);
        }
        LOG_PASS("\r\n");
        ///计算平均值 去掉最大值和最小值
        uint16_t sum = 0;
        uint16_t n = s_pump_stall_adc_value[pump_id].adc_value_count;
        for (int i = 0; i < n; i++) {
            if (s_pump_stall_adc_value[pump_id].adc_value_all[i] != s_pump_stall_adc_value[pump_id].adc_value_max && \
                s_pump_stall_adc_value[pump_id].adc_value_all[i] != s_pump_stall_adc_value[pump_id].adc_value_min) {
                sum += s_pump_stall_adc_value[pump_id].adc_value_all[i];
            }
        }
        if (n > 2) {
            s_pump_stall_adc_value[pump_id].adc_value_avg = sum / (n - 2);
        } else if (n > 0) {
            uint32_t total = 0;
            for (int i = 0; i < n; i++) {
                total += s_pump_stall_adc_value[pump_id].adc_value_all[i];
            }
            s_pump_stall_adc_value[pump_id].adc_value_avg = (uint16_t)(total / n);
        }
        //打印最大值、最小值、平均值
        LOG_PASS("pump %d adc_value_max: %d, adc_value_min: %d, adc_value_avg: %d\r\n", pump_id, s_pump_stall_adc_value[pump_id].adc_value_max, \
            s_pump_stall_adc_value[pump_id].adc_value_min, s_pump_stall_adc_value[pump_id].adc_value_avg);
        u_free(s_pump_stall_adc_value[pump_id].adc_value_all);
        s_pump_stall_adc_value[pump_id].adc_value_all = NULL;
    }
#endif

    return ret;
}

bool pump_motor_is_stalled(uint8_t pump_id)
{
    uint8_t is_stalled = 0;
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump id is out of range: %d", pump_id);
        return false;
    } 
#if IS_SUPPORT_STALL_DETECTION_LOG
    PRINT_LOG("pump %d last_voltage: %d/%d/%d/%d/%d\r\n", pump_id, s_pump_stall_status[pump_id].last_voltage, 
        s_pump_stall_status[pump_id].target_times, s_pump_stall_status[pump_id].work_times,
        s_pump_stall_status[pump_id].sum_voltage, s_pump_stall_status[pump_id].pulse_width);
#endif
    uint16_t threshold = bsp_pump_stall_detection_threshold_calculate(s_pump_stall_status[pump_id].pulse_width);
    if (s_pump_stall_status[pump_id].last_voltage > threshold) {
        is_stalled = true;
    }
    /// todo:是否去抖动
    return is_stalled;
}