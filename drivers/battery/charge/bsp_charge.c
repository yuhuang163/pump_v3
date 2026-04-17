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

#define CHARGE_ABNORMAL_MAX_COUNT           10
#define CHARGE_THREAD_POLL_INTERVAL         100
#define CHARGE_TEMP_POLICY_POLL_INTERVAL    3
/*
* LOCAL VARIABLE DEFINITIONS)
*****************************************************************************************
*/
static uint16_t s_charge_abnormal_count = 0;
static bool s_charge_is_ready = false;
static bool is_charging = false;
static charge_chip_status_t s_charge_status = {0};
static uint8_t s_charge_full_debounce = 0;
static uint8_t s_charge_temp_status = 0;
static uint16_t s_charge_temp_count = 0;
static uint16_t s_charging_current_max = BATT_INPUT_CURRENT_LIMIT;
static uint16_t s_charging_current = 0;
static charge_event_cb s_charge_event_callback = NULL;
static thread_context_t s_charge_device_thd  = {0};
static os_timer_id s_charge_timer_id;
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
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
    pinmux.pin = (enum HAL_IOMUX_PIN_T)CHARGE_SLEEP_PIN;
    hal_iomux_init(&pinmux, 1);
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
    hal_gpio_pin_set_dir(CHARGE_SLEEP_PIN, HAL_GPIO_DIR_OUT, 0);
    #else
    hal_gpio_pin_set_dir(CHARGE_SLEEP_PIN, HAL_GPIO_DIR_OUT, 1);
#endif
#endif
}

static void charge_power_control(bool enable)
{
    GPIO_PIN_CTRL(CHARGE_POWER_PIN, enable ? 0 : 1);
    is_charging = enable ? true:false;
    LOG_PBUS("charge_power_control: %d", enable);
}

static void charge_sleep_control(bool enable)
{
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)   
    GPIO_PIN_CTRL(CHARGE_SLEEP_PIN, enable ? 1 : 0);
#else
    GPIO_PIN_CTRL(CHARGE_SLEEP_PIN, enable ? 0 : 1);
#endif
    LOG_PBUS("charge_sleep_control: %d", enable);
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

static int charge_ibat_config(uint16_t current)
{
    uint8_t reg_data = current / BATT_CURRENT_LIMIT_SCALE;
    s_charging_current = current;
    int ret = charge_write_nbyte(CHARGE_IBAT_CHG_REG, &reg_data, 1);    
    LOG_PBUS("charge_ibat_config: %dmA, ret: %d", current, ret);
    return ret;
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
    return charge_write_nbyte(CHARGE_ADC_SCALE_REG, reg_data, 2);
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
    status->vbat = CHARGE_GET_10BIT_DATA(data_ptr) * BATT_VOLTAGE_SCALE;
    data_ptr += 2;
    status->ibat = CHARGE_GET_10BIT_DATA(data_ptr) * BATT_ADC_CURRENT_LIMIT_SCALE;
    data_ptr += 2;
    status->ntc_resistance  = CHARGE_GET_10BIT_DATA(data_ptr) * NTC_VOLTAGE_SCALE / NTC_CURRENT_COEFF;
    status->status = ((reg_data[10] & 0xFFFF) << 8) | reg_data[11];
    return BSP_OK;
}

static int charge_ntc_temperature_get(float resistance)
{
    int left = 0;
    int right = NTC_TEMP_TABLE_SIZE - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (resistance >= ntc_temp_table[mid].r_min && resistance <= ntc_temp_table[mid].r_max) {
            return ntc_temp_table[mid].temp;
        }
        if (resistance > ntc_temp_table[mid].r_max) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    if (left >= NTC_TEMP_TABLE_SIZE) {
        return ntc_temp_table[NTC_TEMP_TABLE_SIZE - 1].temp + 1;
    }
    return ntc_temp_table[0].temp - 1;
}

static void charge_chip_init(void)
{
    int ret = BSP_OK;
    msleep(20);
    charge_sleep_control(true);
    msleep(20);
    ret = charge_device_is_ready();
    if (ret != BSP_OK) {
        LOGE("charge device is not ready, ret: %d", ret);
        return;
    }

    ret = charge_scale_config();
    if (ret != BSP_OK) {
        LOGE("charge scale config failed, ret: %d", ret);
        return;
    }
    ret = charge_adc_config(true); 
    if (ret != BSP_OK) {
        LOGE("charge adc enable failed, ret: %d", ret);
        return;
    }
    ret = charge_vin_config();
    if (ret != BSP_OK) {
        LOGE("charge vin config failed, ret: %d", ret);
        return;
    }
    ret = charge_vbat_config();
    if (ret != BSP_OK) {
        LOGE("charge vbat config failed, ret: %d", ret);
        return;
    }
    LOG_PBUS("charge chip init success");
}

POSSIBLY_UNUSED static void charging_temp_policy_handler(int8_t temp)
{
    if (temp >= CHARGE_NORMAL_TEMP_MIN && temp <= CHARGE_NORMAL_TEMP_MAX) {
        if (s_charge_temp_status != CHARGE_TEMP_NORMAL) {
            s_charge_temp_status = CHARGE_TEMP_NORMAL;
            s_charge_temp_count = 0;
        }
        s_charge_temp_count++;
        if (s_charge_temp_count > CHARGE_TEMP_POLICY_POLL_INTERVAL) {
            s_charge_temp_count = 0;
            if (s_charging_current != s_charging_current_max) {
                charge_ibat_config(s_charging_current_max);
                LOG_PBUS("normal current update:%dmA", s_charging_current);
            }
        }
    } else if ((temp > CHARGE_DERATED_TEMP_MIN && temp< CHARGE_NORMAL_TEMP_MIN) ||
               (temp> CHARGE_NORMAL_TEMP_MAX && temp < CHARGE_DERATED_TEMP_MAX)) {
        if (s_charge_temp_status != CHARGE_TEMP_DERATED) {
            s_charge_temp_status = CHARGE_TEMP_DERATED;
            s_charge_temp_count = 0;
        }
        s_charge_temp_count++;
        if (s_charge_temp_count > CHARGE_TEMP_POLICY_POLL_INTERVAL) {
            s_charge_temp_count = 0;
            if (s_charging_current != TEMP_ABNORMAL_CHARGING_CURRENT) {
                charge_ibat_config(TEMP_ABNORMAL_CHARGING_CURRENT);
                LOG_PBUS("derated current update:%dmA", s_charging_current);
            }
        }
    } else {
        if (s_charge_temp_status != CHARGE_TEMP_DISABLE) {
            s_charge_temp_status = CHARGE_TEMP_DISABLE;
            s_charge_temp_count = 0;
        }
        s_charge_temp_count++;
        if (s_charge_temp_count > CHARGE_TEMP_POLICY_POLL_INTERVAL) {
            s_charge_temp_count = 0;
            if (is_charging) {
                charge_power_control(false);
                LOG_PBUS("temp abnormal disable charing");
            }
        }
    }
}
#endif

static void charge_device_thread(void *arg)
{
    uint8_t msg = 0;
    uint32_t log_timer = 0;
    uint16_t test_count = 0;
    while (1) {
        MSG_QUEUE_RECV(s_charge_device_thd.msg_id, &msg, OS_WAIT_FOREVER);
#if IS_SUPPORT_PD_CHARGE
        charge_pd_status_poll();
#endif
#if IS_SUPPORT_DEVICE_CHARGE
        charge_status_poll(&s_charge_status);
        s_charge_status.charge_full = (s_charge_status.status & BAT_FULL_STATUS_POS) > 0 ? true : false;        
        s_charge_status.temp = charge_ntc_temperature_get(s_charge_status.ntc_resistance);
#if IS_SUPPORT_CHARGE_CURRENT_ADJUST
        charging_temp_policy_handler(s_charge_status.temp);
#endif
        if (get_timestamp_ms(log_timer) > 2000) {
            log_timer = get_timestamp();
            BSP_LOG("charge status", "vbus:%d mV, ibus:%d mA, vbat:%d mV, ibat:%d mA, ntc:%.2f KOhm, temp:%d C, status:0x%04X", 
                     s_charge_status.vbus, s_charge_status.ibus, s_charge_status.vbat, s_charge_status.ibat, 
                     s_charge_status.ntc_resistance, s_charge_status.temp, s_charge_status.status);
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
    if (s_charge_is_ready == false) {
        return CHARGE_STATE_ERROR;
    }

    if (s_charge_status.charge_full && s_charge_status.plug_in) {
        return CHARGE_STATE_FULL;
    } else if (s_charge_status.plug_in) {
        return CHARGE_STATE_CHARGING;
    } else {
        return CHARGE_STATE_UNCHARGE;
    }
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
#if IS_SUPPORT_PD_CHARGE
    bsp_charge_pd_init();
#endif
#if IS_SUPPORT_DEVICE_CHARGE
    charge_power_gpio_init();
    bsp_i2c_init(CHARGE_I2C_ID);
    charge_chip_init();
    charge_status_poll(&s_charge_status);
    s_charge_is_ready = true;
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
#if IS_SUPPORT_PD_CHARGE
        int current = charge_pd_current_get();
        LOG_PBUS("pd charge imax:%dmA", current);
        if (current > BATT_INPUT_CURRENT_LIMIT) {
            current = BATT_INPUT_CURRENT_LIMIT;
        }
        s_charging_current_max = current;
#if IS_SUPPORT_CHARGE_CURRENT_ADJUST
        charge_ibat_config(current);
#endif
#endif 
        s_charge_status.plug_in = true;      
        charge_power_control(true);
    } else if (event == CHARGE_EVENT_PLUG_OUT) {
        s_charge_status.plug_in = false;
        charge_power_control(false);

    }
#endif

    if (s_charge_event_callback) {
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