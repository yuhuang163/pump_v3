/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_config.h"
#include "hal_gpadc.h"
#include "bsp_light_sensor_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_LIGHT_SENSOR_DBG_PRINT          (0)
#define IS_ADC_FILTER_ENABLE                       (0)  /// 是否启用ADC滤波
#define ADC_FILTER_TIMES                           (4)  /// ADC滤波次数

typedef struct {
    uint8_t target_times;  /// 目标次数
    uint8_t work_times;   /// 工作次数
    uint16_t last_voltage; /// 最后一次电压
    uint32_t sum_voltage;  /// 总电压
} light_sensor_result_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static light_sensor_result_t s_light_sensor = {0};
static rt_device_node_t *node = NULL;
#if IS_SUPPORT_LIGHT_SENSOR_DBG_PRINT
static uint16_t light_sensor_log_count = 0;
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void light_sensor_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    s_light_sensor.work_times++;
    s_light_sensor.sum_voltage += volt;
    if (s_light_sensor.work_times >= s_light_sensor.target_times) {
        hal_gpadc_close(LIGHT_ADC_CHANNEL);
        s_light_sensor.last_voltage = s_light_sensor.work_times > 0 ? \
            s_light_sensor.sum_voltage / s_light_sensor.work_times : 0;
    }
}

static int light_sensor_get_voltage(uint32_t *light_voltage)
{
#if IS_SUPPORT_LIGHT_SENSOR_DBG_PRINT
    uint32_t start_time = get_timestamp();
#endif
    *light_voltage = s_light_sensor.last_voltage;
    if (s_light_sensor.work_times < s_light_sensor.target_times) {
        LOGE("light sensor work times: %d", s_light_sensor.work_times);
        return BSP_OK;
    }
    s_light_sensor.work_times   = 0;
    s_light_sensor.sum_voltage  = 0;
#if IS_ADC_FILTER_ENABLE
    s_light_sensor.target_times = ADC_FILTER_TIMES;
    int ret = hal_gpadc_open(LIGHT_ADC_CHANNEL, HAL_GPADC_ATP_125US, light_sensor_irq_handler);
#else
    s_light_sensor.target_times = 1;
    int ret = hal_gpadc_open(LIGHT_ADC_CHANNEL, HAL_GPADC_ATP_ONESHOT, light_sensor_irq_handler);
#endif
    if (ret != 0) {
        LOGE("gpadc open failed: %d", ret);
        return ret;
    }
#if IS_SUPPORT_LIGHT_SENSOR_DBG_PRINT
    light_sensor_log_count++;
    if (light_sensor_log_count % 20 == 0) {
        LOG_PBUS("voltage:%d, time: %d", *light_voltage, get_timestamp_ms(start_time));
    }
#endif

    return BSP_OK;
}

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int light_sensor_dev_open(struct rt_device_node *dev)
{
    uint32_t tmp;
    light_sensor_get_voltage(&tmp);
    return 0;
}

static int light_sensor_dev_read(struct rt_device_node *dev, void *buf, uint32_t n_bytes)
{
    uint32_t *tmp = (uint32_t *)buf;
    EXCEPTION_CHECK_INT(tmp);
    return light_sensor_get_voltage(tmp);
}

static int light_sensor_dev_suspend(struct rt_device_node *dev)
{
    // hal_gpadc_close(LIGHT_ADC_CHANNEL);
    return 0;
}

static int light_sensor_dev_resume(struct rt_device_node *dev)
{
    uint32_t tmp;
    memset(&s_light_sensor, 0, sizeof(light_sensor_result_t));
    light_sensor_get_voltage(&tmp);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_device_for_light_sensor(list_head_t *list_head)
{
    node = u_malloz(sizeof(rt_device_node_t));
    node->name = "light_sensor";
    node->id = ID_DEVICE_LIGHT_SENSOR;
    node->open = light_sensor_dev_open;
    node->suspend = light_sensor_dev_suspend;
    node->resume = light_sensor_dev_resume;
    node->read = light_sensor_dev_read;
    __list_add_tail(&node->list, list_head);
    LOG_PBUS("dev: %s registered", node->name);

    return node;
}

void light_sensor_test(void)
{
    uint32_t tmp;
    light_sensor_get_voltage(&tmp);
}
