/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_config.h"
#include "bsp_i2c.h" // IWYU pragma: keep
#include "bsp_charge.h"
#include "bsp_ntc_map.h" // IWYU pragma: keep
#include "bsp_battery_internal.h"
#if IS_SUPPORT_PD_CHARGE
#include "bsp_charge_pd.h"
#endif


/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_CHARGE_CURRENT_ADJUST    1
#define CHARGING_STATUS_VALUE               0x02
#define CHARHE_FULL_STATUS_VALUE            0x01
#define UNCHARGE_STATUS_VALUE               0x00 

#define CHARHE_CHECK_NO_BATT_0              0xFF 
#define CHARHE_CHECK_NO_BATT_1              0xFE 
#define CHARGE_I2C_ADDR                     0x75
#define CHARGE_RETRY_COUNT                  3
#define CHARGE_GET_10BIT_DATA(data)         (((data[1] & 0xFF03) << 8) | data[0])

#define CHARGE_ABNORMAL_MAX_COUNT           20
#define CHARGE_THREAD_POLL_INTERVAL         100
#define CHARGE_TEMP_POLICY_POLL_INTERVAL    3
#define CHARGE_VBAT_AVG_WINDOW              20   /// VBAT 采样滑动平均窗口（100ms 轮询 × 8 = 800ms）
#define CHARGE_TEMP_AVG_WINDOW              20   /// NTC 温度滑动平均窗口（100ms 轮询 × 20 = 2s）
/// 温度策略模拟：1 打开后用假温度驱动充电电流，量产必须为 0
#define CHARGE_TEMP_POLICY_SIM_TEST         0
#if CHARGE_TEMP_POLICY_SIM_TEST
/// 1: 每轮随机洗牌（每个点仍测到一次）；0: 固定顺序循环
#define CHARGE_TEMP_SIM_RANDOM              1
/// 策略落地后再保持的拍数（100ms/拍），方便看 ibat；0 表示一落地就切下一档
#define CHARGE_TEMP_SIM_HOLD_AFTER_LAND     40
#define CHARGE_TEMP_SIM_POOL_LEN            14
#endif
/*
* LOCAL VARIABLE DEFINITIONS)
*****************************************************************************************
*/
static uint16_t s_charge_abnormal_count = 0;
static bool s_charge_is_ready = false;
static bool is_charging = false;
static bool s_charge_adapter_mismatch = false;
static charge_chip_status_t s_charge_status = {0};
static uint8_t s_charge_full_debounce = 0;
static uint8_t s_charge_temp_status = 0;
static uint16_t s_charge_temp_count = 0;
static uint16_t s_charging_current_max = BATT_INPUT_CURRENT_LIMIT;
static uint16_t s_charging_current = 0;
static charge_event_cb s_charge_event_callback = NULL;
static thread_context_t s_charge_device_thd  = {0};
static os_timer_id s_charge_timer_id;
/// VBAT 滑动平均滤波
static uint16_t s_vbat_window[CHARGE_VBAT_AVG_WINDOW] = {0};
static uint32_t s_vbat_window_sum = 0;
static uint8_t  s_vbat_window_cnt = 0;
static uint8_t  s_vbat_window_idx = 0;
/// NTC 温度滑动平均滤波
static float    s_temp_window[CHARGE_TEMP_AVG_WINDOW] = {0};
static float    s_temp_window_sum = 0;
static uint8_t  s_temp_window_cnt = 0;
static uint8_t  s_temp_window_idx = 0;
static const char *charge_status_arr[] = {
    "charge error",
    "charge full",
    "charging", 
    "uncharge",
    "no battery",
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_DEVICE_CHARGE 
static int charge_write_nbyte(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;

    if (s_charge_is_ready == 0) {
        LOGE("charge is not ready");
        return BSP_NOT_READY;
    }

#if CHARGE_I2C_ID == TOUCH_KEY_I2C_ID
    if (touch_key_is_upgrading()) {
        return BSP_OK;
    }
#endif

    i2c_master_data_t i2c_data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    for (uint8_t i = 0; i < CHARGE_RETRY_COUNT; i++) {
        ret = bsp_i2c_write_byte(CHARGE_I2C_ID, &i2c_data);
        if (ret == BSP_OK) {
            s_charge_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_charge_abnormal_count++;
    if (s_charge_abnormal_count > CHARGE_ABNORMAL_MAX_COUNT) {
        s_charge_is_ready = 0;
    }

    return BSP_ERROR;
}

static int charge_read_nbyte(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;
    
    if (s_charge_is_ready == 0) {
        // LOGE("charge is not ready");
        // return BSP_NOT_READY;
    }

#if CHARGE_I2C_ID == TOUCH_KEY_I2C_ID
    if (touch_key_is_upgrading()) {
        return BSP_OK;
    }
#endif

    i2c_master_data_t i2c_data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    for (uint8_t i = 0; i < CHARGE_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(CHARGE_I2C_ID, &i2c_data);
        if (ret == BSP_OK) {
            s_charge_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_charge_abnormal_count++;
    if (s_charge_abnormal_count > CHARGE_ABNORMAL_MAX_COUNT) {
        s_charge_is_ready = 0;
    }

    return BSP_ERROR;
}

static int charge_read_nbyte_no_retry(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;

#if CHARGE_I2C_ID == TOUCH_KEY_I2C_ID
    if (touch_key_is_upgrading()) {
        return BSP_OK;
    }
#endif

    i2c_master_data_t i2c_data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    ret = bsp_i2c_read_byte(CHARGE_I2C_ID, &i2c_data);
    if (ret == BSP_OK) {
        s_charge_abnormal_count = 0;
        return BSP_OK;
    }

    s_charge_abnormal_count++;
    if (s_charge_abnormal_count > CHARGE_ABNORMAL_MAX_COUNT) {
        s_charge_is_ready = 0;
    }

    return BSP_ERROR;
}

static void charge_power_gpio_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux = {
        (enum HAL_IOMUX_PIN_T)CHARGE_POWER_PIN, 
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&pinmux, 1);
    hal_gpio_pin_set_dir(CHARGE_POWER_PIN, HAL_GPIO_DIR_OUT, 1);
    pinmux.pin = (enum HAL_IOMUX_PIN_T)CHARGE_SLEEP_PIN;
    hal_iomux_init(&pinmux, 1);
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
    hal_gpio_pin_set_dir(CHARGE_SLEEP_PIN, HAL_GPIO_DIR_OUT, 0);
#else
    hal_gpio_pin_set_dir(CHARGE_SLEEP_PIN, HAL_GPIO_DIR_OUT, 1);
#endif
}

static void charge_power_control(bool enable)
{
    if (is_charging == enable) {
        return;
    }
    
    if (s_charge_is_ready == 0) {
        LOGE("charge no ready, ignore");
        return;
    }
    GPIO_PIN_CTRL(CHARGE_POWER_PIN, enable ? 0 : 1);
    is_charging = enable;
    LOG_PBUS("charge_power_control: %d", enable);
}

static void charge_sleep_control(bool enable)
{
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)   
    GPIO_PIN_CTRL(CHARGE_SLEEP_PIN, enable ? 1 : 0);
#else
    GPIO_PIN_CTRL(CHARGE_SLEEP_PIN, enable ? 0 : 1);
#endif
}

static int charge_device_is_ready(void)
{
    int ret = 0;
    uint8_t value = 0;
    for (uint8_t i = 0; i < 5; i++) {
        ret = charge_read_nbyte_no_retry(CHARGE_ID_REG, &value, 1);
        if (value == CHARGE_ID_VALUE) {            
            s_charge_is_ready = true;
            s_charge_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(20);
    }
    return BSP_ERROR;
}

static int charge_vbat_config(void)
{
    uint8_t reg_data = 0;
    uint8_t reg_addr = CHARGE_VBAT_SET_REG;
    reg_data = VCELL_4_2V | CSEL_3_CELL | IRCOMP_0MOHM;
    if (charge_write_nbyte(CHARGE_VBAT_SET_REG, &reg_data, 1) != BSP_OK) {
        return BSP_ERROR;
    }
    /// 设置充电电流限制
#if IS_SUPPORT_CHARGE_CURRENT_ADJUST
    s_charging_current = BATT_INPUT_CURRENT_LIMIT;
#else
    s_charging_current = 1500;
#endif
    reg_data = s_charging_current / BATT_CURRENT_LIMIT_SCALE; /// 1000mA
    if (charge_write_nbyte(CHARGE_IBAT_CHG_REG, &reg_data, 1) != BSP_OK) {
        return BSP_ERROR;
    }
    LOG_PBUS("charge_vbat_config: %dmA", s_charging_current);
    /// 设置放电电流限制
    reg_data = BATT_OUTPUT_CURRENT_LIMIT / BATT_CURRENT_LIMIT_SCALE; /// 3000mA
    if (charge_write_nbyte(CHARGE_IBAT_DCHG_REG, &reg_data, 1) != BSP_OK) {
        return BSP_ERROR;
    }
    return BSP_OK;
}

static int charge_ibat_config_impl(uint16_t current, bool force)
{
    uint8_t reg_data = current / BATT_CURRENT_LIMIT_SCALE;
    if (!force && s_charging_current == current) {
        return BSP_OK;
    }
    int ret = charge_write_nbyte(CHARGE_IBAT_CHG_REG, &reg_data, 1);
    if (ret == BSP_OK) {
        s_charging_current = current;
    }
    LOG_PBUS("charge_ibat_config: %dmA, ret: %d", current, ret);
    return ret;
}

static int charge_ibat_config(uint16_t current)
{
    return charge_ibat_config_impl(current, false);
}

static int charge_vin_config(void)
{
    uint8_t reg_data[2] = {0};
    /// 输入电压限制
    reg_data[0] = 0; 
    reg_data[1] = CHARGE_INPUT_VOLTAGE_LIMIT / CHARGE_INPUT_VOLTAGE_LIMIT_SCALE; /// 低于4.2V，启用限流
    if (charge_write_nbyte(CHARGE_VINDPM_H_REG, reg_data, 2) != BSP_OK) {
        return BSP_ERROR;
    }   
    /// 输入电流限制
    reg_data[0] = CHARGE_INPUT_CURRENT_LIMIT / CHARGE_INPUT_CURRENT_LIMIT_SCALE; /// 3000mA
    return charge_write_nbyte(CHARGE_IIN_LIMIT_REG, reg_data, 1);
}

static int charge_adc_config(uint8_t adc_en)
{
    uint8_t adc_set = 0;
    if (adc_en) {
        adc_set = AD_START_ENABLE | ADC_VBUS_ENABLE | ADC_IBUS_ENABLE |\
         ADC_VBAT_ENABLE | ADC_IBAT_ENABLE | ADC_NTC_ENABLE;
    }
    return charge_write_nbyte(CHARGE_ADC_SET_REG, &adc_set, 1);
}

static int charge_scale_config(void)
{
    uint8_t scale = 0;
    scale = VBUS_SCALE_CFG | IBUS_SCALE_CFG | VBAT_SCALE_CFG | IBAT_SCALE_CFG;
    return charge_write_nbyte(CHARGE_ADC_SCALE_REG, &scale, 1);
}

static int charge_interrupt_config(void)
{
    uint8_t reg_data[2] = {0};
    reg_data[0] = CHARGE_AC_PRESENT_INT_ENABLE | CHARGE_INDET_INT_ENABLE | CHARGE_BUSUVP_CHG_INT_ENABLE | 
        CHARGE_BATOVP_INT_ENABLE | CHARGE_BAT_FULL_INT_ENABLE | CHARGE_RE_CHG_INT_ENABLE;
    reg_data[1] = CHARGE_CHG_TIMEOUT_INT_ENABLE | CHARGE_TSHUT_INT_ENABLE | CHARGE_BAT_EMPTY_INT_ENABLE | 
        CHARGE_NTC_FAULT_INT_ENABLE;
    return charge_write_nbyte(CHARGE_INT_EN0_REG, reg_data, 2);
}

POSSIBLY_UNUSED static int charge_ctrl0_config(void)
{
    uint8_t reg_data = 0;
    reg_data = PWM_FREQ_DEFAULT | EN_DITHER_ENABLE;
    return charge_write_nbyte(CHARGE_CTRL0_REG, &reg_data, 1);
}

/// VBAT 采集偏差补偿：对实时采样电压做修正（补偿策略未定，当前直接透传）。
/// 后续标定/分段补偿在此实现，vbat 平均值基于本函数输出计算。
static uint16_t charge_vbat_compensate(uint16_t raw_mv)
{
    // TODO: 接入两点标定 / 分段补偿
    return raw_mv;
}

/// VBAT 滑动平均：对连续 CHARGE_VBAT_AVG_WINDOW 次电压值（补偿后）求均值，滤除抖动。
/// 窗口未满时按已填充样本数取均值，上电即平滑输出，无需等待满窗。
static uint16_t charge_vbat_average(uint16_t mv)
{
    uint16_t old = s_vbat_window[s_vbat_window_idx];
    s_vbat_window[s_vbat_window_idx] = mv;
    s_vbat_window_sum += mv;
    s_vbat_window_sum -= old;
    s_vbat_window_idx = (uint8_t)((s_vbat_window_idx + 1) % CHARGE_VBAT_AVG_WINDOW);
    if (s_vbat_window_cnt < CHARGE_VBAT_AVG_WINDOW) {
        s_vbat_window_cnt++;
    }
    return (uint16_t)(s_vbat_window_sum / s_vbat_window_cnt);
}

/// NTC 温度滑动平均：对连续 CHARGE_TEMP_AVG_WINDOW 次温度求均值，滤除抖动。
/// 窗口未满时按已填充样本数取均值，上电即平滑输出，无需等待满窗。
static float charge_temp_average(float temp)
{
    float old = s_temp_window[s_temp_window_idx];
    s_temp_window[s_temp_window_idx] = temp;
    s_temp_window_sum += temp;
    s_temp_window_sum -= old;
    s_temp_window_idx = (uint8_t)((s_temp_window_idx + 1) % CHARGE_TEMP_AVG_WINDOW);
    if (s_temp_window_cnt < CHARGE_TEMP_AVG_WINDOW) {
        s_temp_window_cnt++;
    }
    return s_temp_window_sum / s_temp_window_cnt;
}

static int charge_status_poll(charge_chip_status_t *status)
{
    uint8_t reg_data[12] = {0};
    int ret = charge_read_nbyte(CHARGE_ADC_VBUS_L_REG, reg_data, 12);
    if (ret != BSP_OK) {
        LOGE("charge adc poll failed, ret: %d", ret);
        return ret;
    }
    uint8_t *data_ptr = reg_data;
    status->vbus = CHARGE_GET_10BIT_DATA(data_ptr) * CHARGE_INPUT_VOLTAGE_LIMIT_SCALE;
    data_ptr += 2;
    status->ibus = CHARGE_GET_10BIT_DATA(data_ptr) * CHARGE_ADC_CURRENT_LIMIT_SCALE;
    data_ptr += 2;
    uint16_t vbat_code = CHARGE_GET_10BIT_DATA(data_ptr);
    status->vbat_raw = vbat_code * BATT_VOLTAGE_SCALE;                   // 实时值（未滤波）
    status->vbat_comp = charge_vbat_compensate(status->vbat_raw);        // 补偿后值（暂等于实时值）
    status->vbat = charge_vbat_average(status->vbat_comp);               // 平均值（对补偿后值求平均）
    data_ptr += 2;
    status->ibat = CHARGE_GET_10BIT_DATA(data_ptr) * BATT_ADC_CURRENT_LIMIT_SCALE;
    data_ptr += 2;
    status->ntc_resistance  = CHARGE_GET_10BIT_DATA(data_ptr) * NTC_VOLTAGE_SCALE / NTC_CURRENT_COEFF;
    status->status = ((reg_data[10] & 0xFFFF) << 8) | reg_data[11];
    return BSP_OK;
}

static float charge_ntc_temperature_get(float resistance)
{
    int left = 0;
    int right = NTC_TEMP_TABLE_SIZE - 1;
    int mid;

    if (resistance >= ntc_temp_table[left].r_center) {
        return (float)ntc_temp_table[left].temp;
    }

    if (resistance <= ntc_temp_table[right].r_center) {
        return (float)ntc_temp_table[right].temp;
    }

    while (left < right - 1)
    {
        mid = left + (right - left) / 2; // 防止溢出

        if (resistance > ntc_temp_table[mid].r_center)
        {
            right = mid;
        }
        else
        {
            left = mid;
        }
    }

    float R_high = ntc_temp_table[left].r_center;
    float T_high = (float)ntc_temp_table[left].temp;
    float R_low  = ntc_temp_table[right].r_center;
    float T_low  = (float)ntc_temp_table[right].temp;

    float temp = T_high + (T_low - T_high) * (R_high - resistance) / (R_high - R_low);

    return temp;
}

static int charge_chip_init(void)
{
    int ret = BSP_OK;
    msleep(20);
    charge_sleep_control(true);
    msleep(20);
    ret = charge_device_is_ready();
    if (ret != BSP_OK) {
        LOGE("charge device is not ready, ret: %d", ret);
        return ret;
    }

    ret = charge_scale_config();
    if (ret != BSP_OK) {
        LOGE("charge scale config failed, ret: %d", ret);
        return ret;
    }
    ret = charge_adc_config(true); 
    if (ret != BSP_OK) {
        LOGE("charge adc enable failed, ret: %d", ret);
        return ret;
    }
    ret = charge_vin_config();
    if (ret != BSP_OK) {
        LOGE("charge vin config failed, ret: %d", ret);
        return ret;
    }
    ret = charge_vbat_config();
    if (ret != BSP_OK) {
        LOGE("charge vbat config failed, ret: %d", ret);
        return ret;
    }

#if 0 /// for test     
    ret = charge_ctrl0_config();
    if (ret != BSP_OK) {
        LOGE("charge ctrl0 config failed, ret: %d", ret);
        return ret;
    }
#endif
    LOG_PBUS("charge chip init success");
    return BSP_OK;
}

#if IS_SUPPORT_CHARGE_CURRENT_ADJUST
static charging_temp_status_e charging_temp_policy_target(float temp)
{
    if (temp <= CHARGE_DERATED_TEMP_MIN || temp >= CHARGE_DERATED_TEMP_MAX) {
        return CHARGE_TEMP_DISABLE;
    }
    if (temp >= CHARGE_NORMAL_TEMP_MIN && temp <= CHARGE_NORMAL_TEMP_MAX) {
        return CHARGE_TEMP_NORMAL;
    }
    return CHARGE_TEMP_DERATED;
}

static uint16_t charging_temp_policy_ibat(charging_temp_status_e status)
{
    if (status == CHARGE_TEMP_NORMAL) {
        return s_charging_current_max;
    }
    
    if (status == CHARGE_TEMP_DERATED) {
        if (s_charging_current_max < TEMP_ABNORMAL_CHARGING_CURRENT) {
            return s_charging_current_max;
        }
        return TEMP_ABNORMAL_CHARGING_CURRENT;
    }
    return 0;
}

static void charging_temp_policy_apply(charging_temp_status_e target)
{
    if (s_charge_temp_status == target) {
        if (target == CHARGE_TEMP_DISABLE) {
            charge_power_control(false);
        } else {
            uint16_t expect = charging_temp_policy_ibat(target);
            if (s_charging_current != expect || s_charge_status.ibat > expect) {
                charge_ibat_config_impl(expect, s_charge_status.ibat > expect);
            }
        }
        s_charge_temp_count = 0;
        return;
    }

    s_charge_temp_count++;
    if (s_charge_temp_count <= CHARGE_TEMP_POLICY_POLL_INTERVAL) {
        return;
    }
    s_charge_temp_count = 0;
    uint16_t expect = charging_temp_policy_ibat(target);
    switch (target) {
        case CHARGE_TEMP_DISABLE:
            charge_power_control(false);
            s_charge_temp_status = CHARGE_TEMP_DISABLE;
            LOG_PBUS("temp abnormal disable charing, expect: %dmA", expect);
            break;
        case CHARGE_TEMP_NORMAL:
            charge_ibat_config(expect);
            if (s_charge_status.plug_in && s_charge_adapter_mismatch == false) {
                charge_power_control(true);
            }
            s_charge_temp_status = CHARGE_TEMP_NORMAL;
            LOG_PBUS("goto normal charging, expect: %dmA", expect);
            break;
        case CHARGE_TEMP_DERATED:
            charge_ibat_config(expect);
            if (s_charge_status.plug_in && s_charge_adapter_mismatch == false) {
                charge_power_control(true);
            }
            s_charge_temp_status = CHARGE_TEMP_DERATED;
            LOG_PBUS("goto temp derated charging, expect: %dmA", expect);
            break;
        default:
            break;
    }
}

static void charging_temp_policy_handler(float temp)
{
    charging_temp_policy_apply(charging_temp_policy_target(temp));
}

#if CHARGE_TEMP_POLICY_SIM_TEST
/// 覆盖停充 / 600mA / 协议电流，以及 0/15/45/60 端点
static const float s_charge_temp_sim_pool[CHARGE_TEMP_SIM_POOL_LEN] = {
    -10.0f, -0.1f,  0.0f,
      0.1f,  8.0f, 14.9f,
     15.0f, 25.0f, 45.0f,
     45.1f, 50.0f, 59.9f,
     60.0f, 62.0f,
};
static float s_charge_temp_sim_order[CHARGE_TEMP_SIM_POOL_LEN];
static uint8_t s_charge_temp_sim_idx;
static uint8_t s_charge_temp_sim_hold;
static uint8_t s_charge_temp_sim_inited;
static uint32_t s_charge_temp_sim_rng;

static uint32_t charge_temp_sim_rand(void)
{
    s_charge_temp_sim_rng = s_charge_temp_sim_rng * 1103515245u + 12345u;
    return s_charge_temp_sim_rng >> 16;
}

static void charge_temp_sim_start_round(void)
{
    uint8_t i;

    for (i = 0; i < CHARGE_TEMP_SIM_POOL_LEN; i++) {
        s_charge_temp_sim_order[i] = s_charge_temp_sim_pool[i];
    }
#if CHARGE_TEMP_SIM_RANDOM
    for (i = CHARGE_TEMP_SIM_POOL_LEN - 1; i > 0; i--) {
        uint8_t j = (uint8_t)(charge_temp_sim_rand() % (i + 1));
        float tmp = s_charge_temp_sim_order[i];
        s_charge_temp_sim_order[i] = s_charge_temp_sim_order[j];
        s_charge_temp_sim_order[j] = tmp;
    }
#endif
    s_charge_temp_sim_idx = 0;
    s_charge_temp_sim_hold = 0;
    LOG_PBUS("charge temp sim round, first %.1fC", s_charge_temp_sim_order[0]);
}

static float charge_temp_policy_sim_get(void)
{
    if (s_charge_temp_sim_inited == 0) {
        s_charge_temp_sim_rng = get_timestamp();
        if (s_charge_temp_sim_rng == 0) {
            s_charge_temp_sim_rng = 1;
        }
        charge_temp_sim_start_round();
        s_charge_temp_sim_inited = 1;
    }
    return s_charge_temp_sim_order[s_charge_temp_sim_idx];
}

static void charge_temp_policy_sim_advance_if_landed(void)
{
    float temp = s_charge_temp_sim_order[s_charge_temp_sim_idx];
    charging_temp_status_e expect = charging_temp_policy_target(temp);

    if (s_charge_temp_status != expect) {
        s_charge_temp_sim_hold = 0;
        return;
    }

    if (s_charge_temp_sim_hold < CHARGE_TEMP_SIM_HOLD_AFTER_LAND) {
        s_charge_temp_sim_hold++;
        return;
    }

    s_charge_temp_sim_idx++;
    if (s_charge_temp_sim_idx >= CHARGE_TEMP_SIM_POOL_LEN) {
        charge_temp_sim_start_round();
    } else {
        s_charge_temp_sim_hold = 0;
        LOG_PBUS("charge temp sim: %.1fC expect %d",
                 s_charge_temp_sim_order[s_charge_temp_sim_idx],
                 charging_temp_policy_target(s_charge_temp_sim_order[s_charge_temp_sim_idx]));
    }
}
#endif

static void charging_temp_policy_on_plug_in(void)
{
    if (charge_status_poll(&s_charge_status) == BSP_OK) {
        s_charge_status.temp_raw = charge_ntc_temperature_get(s_charge_status.ntc_resistance);
        s_charge_status.temp = charge_temp_average(s_charge_status.temp_raw);
    }
    s_charge_temp_count = 0;
    s_charge_temp_status = CHARGE_TEMP_UNKNOW;
}
#endif
#endif

static void charge_device_thread(void *arg)
{
    uint8_t msg = 0;
    uint32_t log_timer = 0;
    uint16_t test_count = 0;
    while (1) {
#if IS_SUPPORT_PD_CHARGE
        charge_pd_status_poll(true);
#endif
        MSG_QUEUE_RECV(s_charge_device_thd.msg_id, &msg, OS_WAIT_FOREVER);
#if IS_SUPPORT_DEVICE_CHARGE
        charge_status_poll(&s_charge_status);
        s_charge_status.charge_full = (s_charge_status.status & BAT_FULL_STATUS_POS) > 0 ? true : false;  
        s_charge_status.temp_raw = charge_ntc_temperature_get(s_charge_status.ntc_resistance);
        s_charge_status.temp = charge_temp_average(s_charge_status.temp_raw);
#if IS_SUPPORT_CHARGE_CURRENT_ADJUST
#if CHARGE_TEMP_POLICY_SIM_TEST
        s_charge_status.temp_raw = charge_temp_policy_sim_get();
        s_charge_status.temp = s_charge_status.temp_raw;
        charging_temp_policy_handler(s_charge_status.temp);
        charge_temp_policy_sim_advance_if_landed();
#else
        charging_temp_policy_handler(s_charge_status.temp);
#endif
#endif
        if (get_timestamp_ms(log_timer) > 2000) {
            log_timer = get_timestamp();
            BSP_LOG("charge chip", "vbus:%dmV, ibus:%dmA, vbat:%d(%d/%d)mV, ibat:%dmA, ntc:%.2fKOhm, temp:%.1f(%.1f)%d, status:0x%04X, pd:%d",
                    s_charge_status.vbus, s_charge_status.ibus, 
                    s_charge_status.vbat, s_charge_status.vbat_raw, s_charge_status.vbat_comp, s_charge_status.ibat,
                    s_charge_status.ntc_resistance, s_charge_status.temp, s_charge_status.temp_raw, s_charge_temp_status,
                    s_charge_status.status, get_charge_pd_version());
        }
#endif
    }
}

static void charge_thread_timer_handle(void *arg)
{
    uint8_t msg = 0;
    MSG_QUEUE_SEND(s_charge_device_thd.msg_id, (void *)&msg, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void charge_vbus_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)CHARGE_PD_INT_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(CHARGE_PD_INT_PIN,  HAL_GPIO_DIR_IN,  1);
}

bool vbus_is_plug_in(void)
{
    return hal_gpio_pin_get_val(CHARGE_PD_INT_PIN) == 0 ? true : false;
}

void add_charge_event_callback(charge_event_cb cb)
{
    s_charge_event_callback = cb;
}

int get_charge_state(void) 
{
    if (s_charge_adapter_mismatch && s_charge_status.plug_in) {
        return CHARGE_STATE_ERROR;
    }

    if (s_charge_status.charge_full && s_charge_status.plug_in &&
        is_charging && s_charge_full_debounce >= BATT_CHARGE_FULL_DEBOUNCE) {
        return CHARGE_STATE_FULL;
    }  

    if (s_charge_status.plug_in && is_charging) {
        return CHARGE_STATE_CHARGING;
    }
    
    return CHARGE_STATE_UNCHARGE;
}

int get_charging_temperature(void) 
{
    return s_charge_is_ready ? s_charge_status.temp : 26;
}

int get_charge_battery_voltage(void) 
{
    return s_charge_status.vbat;
}

int get_charge_full_debounce(void) 
{
    if (s_charge_status.charge_full && s_charge_status.plug_in) {
        if (s_charge_full_debounce < BATT_CHARGE_FULL_DEBOUNCE) {
            s_charge_full_debounce++;
        }
    } else {
        s_charge_full_debounce = 0;
    }
    return s_charge_full_debounce;
}

int bsp_charge_init(void)
{
#if IS_SUPPORT_DEVICE_CHARGE
    int ret = BSP_OK;
    charge_power_gpio_init();
    bsp_i2c_init(CHARGE_I2C_ID);
    ret = charge_chip_init();
    if (ret == BSP_OK) {
        msleep(50);
        ret = charge_status_poll(&s_charge_status);
        LOG_PBUS("charge vbat:%dmV, ibat:%dmA, temp:%.1f", s_charge_status.vbat, s_charge_status.ibat, s_charge_status.temp);
    }
    s_charge_is_ready = (ret == BSP_OK);
#endif
#if IS_SUPPORT_PD_CHARGE
    bsp_charge_pd_init();
#endif
    CREATE_MSG_TYPE_QUEUE(&s_charge_device_thd.msg_id, 1, 1);
    CREATE_TIMER(&s_charge_timer_id, "charge", NULL, OS_TIMER_PERIODIC, charge_thread_timer_handle);
    START_TIMER(s_charge_timer_id, CHARGE_THREAD_POLL_INTERVAL);
    RUN_NEW_THREAD(&s_charge_device_thd, "charge", charge_device_thread, RT_PRIO_CHARGE_DEVICE_THREAD, 
        (CHARGE_DEVICE_THREAD_STACKSIZE));

    return BSP_OK;
}

int bsp_charge_resume(void)
{
    s_charge_is_ready = true;
    s_charge_abnormal_count = 0;
    s_charge_temp_count = 0;
    s_charge_temp_status = CHARGE_TEMP_UNKNOW;
    START_TIMER(s_charge_timer_id, CHARGE_THREAD_POLL_INTERVAL);
    return BSP_OK;
}

int bsp_charge_suspend(void)
{
    s_charge_is_ready = false;
    s_charge_abnormal_count = 0;
    s_charge_temp_count = 0;
    s_charge_temp_status = CHARGE_TEMP_UNKNOW;
    STOP_TIMER(s_charge_timer_id);
    return BSP_OK;
}

int bsp_charge_is_ready(void)
{
    return s_charge_is_ready;
}

void charge_event_internal_cb(charge_event_e event)
{
#if IS_SUPPORT_DEVICE_CHARGE  
    if (event == CHARGE_EVENT_PLUG_IN) {
        if (s_charge_status.plug_in && s_charge_adapter_mismatch == false) {
            return;
        }
    #if IS_SUPPORT_PD_CHARGE
        int current = charge_pd_current_get();
        LOG_PBUS("pd charge imax:%dmA", current);
        if (current > BATT_INPUT_CURRENT_LIMIT || 0 == current) {
            current = BATT_INPUT_CURRENT_LIMIT;
        }
        s_charging_current_max = current;
    #endif
        s_charge_status.plug_in = true;
        s_charge_adapter_mismatch = false;
        s_charge_status.charge_full = false;
    #if IS_SUPPORT_CHARGE_CURRENT_ADJUST
        charging_temp_policy_on_plug_in();
    #else
        charge_power_control(true);
    #endif
        LOG_PBUS("charge plug in");
    } else if (event == CHARGE_EVENT_PLUG_OUT) {
        if (s_charge_status.plug_in == false) {
            return;
        }
        s_charge_status.plug_in = false;
        s_charge_adapter_mismatch = false;
        s_charge_status.charge_full = false;
        s_charge_full_debounce = 0;
        charge_power_control(false);
        LOG_PBUS("charge plug out");
    } else if (event == CHARGE_EVENT_ADAPTER_MISMATCH) {
        if (s_charge_adapter_mismatch) {
            return;
        }
        s_charge_status.plug_in = true;
        s_charge_adapter_mismatch = true;
        s_charge_status.charge_full = false;
        s_charge_full_debounce = 0;
        charge_power_control(false);
        LOG_PBUS("charge adapter mismatch");
    }
    else if (event == CHARGE_EVENT_FORCE_PLUG_IN) { //主要用于适配器不适配时，强制触发充电
    #if IS_SUPPORT_PD_CHARGE
        int current = charge_pd_current_get();
        LOG_PBUS("pd charge imax:%dmA", current);
        if (current > BATT_INPUT_CURRENT_LIMIT || 0 == current) {
            current = 3000; /// max-3A
        }
        s_charging_current_max = current;
    #endif
        s_charge_status.plug_in = true;
        s_charge_adapter_mismatch = false;
        s_charge_status.charge_full = false;
    #if IS_SUPPORT_CHARGE_CURRENT_ADJUST
        charging_temp_policy_on_plug_in();
    #else
        charge_power_control(true);
    #endif
        LOG_PBUS("charge plug in");
    }
#else
    if (event == CHARGE_EVENT_PLUG_IN || event == CHARGE_EVENT_PLUG_OUT) {   
        if (s_charge_adapter_mismatch == false) {
            return;
        }
        s_charge_adapter_mismatch = false;
        s_charge_status.plug_in = false;
    } else if (event == CHARGE_EVENT_ADAPTER_MISMATCH) {
        if (s_charge_adapter_mismatch) {
            return;
        }
        s_charge_adapter_mismatch = true;
        s_charge_status.plug_in = true;
    }
#endif
    if (s_charge_event_callback) {
        if (event == CHARGE_EVENT_FORCE_PLUG_IN) {
            event = CHARGE_EVENT_PLUG_IN;
        }
        s_charge_event_callback(event);
    } 
}

void charge_current_adjust(uint16_t current)
{
    if (current > 5000) {
        LOGE("charge current adjust out of range, current: %dmA", current);
        return;
    }
#if IS_SUPPORT_DEVICE_CHARGE 
    charge_ibat_config(current);
#endif
}

int bsp_charge_get_ibat_ma(uint16_t *ibat_ma)
{
    if (ibat_ma == NULL) {
        return BSP_ERROR;
    }
    *ibat_ma = s_charge_status.ibat;
    return BSP_OK;
}
