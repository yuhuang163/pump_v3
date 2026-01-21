/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "bsp_charge.h"
#include "bsp_ntc_map.h"
#include "bsp_battery_internal.h"
#include "cmsis_os_api.h"
#if IS_SUPPORT_PD_CHARGE
#include "bsp_charge_pd.h"
#endif


/*
* DEFINES
*****************************************************************************************
*/
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
/*
* LOCAL VARIABLE DEFINITIONS)
*****************************************************************************************
*/
static uint16_t s_charge_abnormal_count = 0;
static bool s_charge_is_ready = false;
static charge_chip_status_t s_charge_status = {0};
static uint8_t s_charge_full_debounce = 0;
static void (*s_charge_irq_callback)(void *param);
static int (*s_charge_battery_poll_callback)(void);
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
static int charge_write_nbyte(i2c_master_data_t *data)
{
    int ret = 0;

    if (s_charge_is_ready == 0) {
        LOGE("charge is not ready");
        return BSP_NOT_READY;
    }

    for (uint8_t i = 0; i < CHARGE_RETRY_COUNT; i++) {
        ret = bsp_i2c_write_byte(CHARGE_I2C_ID, data);
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

static int charge_read_nbyte(i2c_master_data_t *data)
{
    int ret = 0;
    
    if (s_charge_is_ready == 0) {
        // LOGE("charge is not ready");
        // return BSP_NOT_READY;
    }

    for (uint8_t i = 0; i < CHARGE_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(CHARGE_I2C_ID, data);
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

#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
static void charge_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    uint8_t msg = 0;
    if (pin == CHARGE_INT_PIN && s_charge_device_thd.msg_id) {
        MSG_QUEUE_SEND(s_charge_device_thd.msg_id, &msg, 0);
    }
}

static void charge_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable, 
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, 
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING, 
        charge_irq_callback
    };

    hal_gpio_setup_irq(CHARGE_INT_PIN, &gpio_irq_cfg);
}

static void charge_irq_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)CHARGE_INT_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(CHARGE_INT_PIN,  HAL_GPIO_DIR_IN,  1);
}
#endif

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
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
    pinmux.pin = (enum HAL_IOMUX_PIN_T)CHARGE_SLEEP_PIN;
    hal_iomux_init(&pinmux, 1);
    hal_gpio_pin_set_dir(CHARGE_SLEEP_PIN, HAL_GPIO_DIR_OUT, 1);
#endif
}

static void charge_power_control(bool enable)
{
    GPIO_PIN_CTRL(CHARGE_POWER_PIN, enable ? 0 : 1);
    LOG_PBUS("charge_power_control: %d", enable);
}

static void charge_sleep_control(bool enable)
{
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
    GPIO_PIN_CTRL(CHARGE_SLEEP_PIN, enable ? 0 : 1);
    LOG_PBUS("charge_sleep_control: %d", enable);
#endif
}

static int charge_device_is_ready(void)
{
    int ret = 0;
    uint8_t value = 0;
    i2c_master_data_t read_data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = CHARGE_ID_REG,
        .data = &value,
        .len = 1,
    };

    for (uint8_t i = 0; i < 5; i++) {
        ret = bsp_i2c_read_byte(CHARGE_I2C_ID, &read_data);
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
    i2c_master_data_t data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = CHARGE_VBAT_SET_REG,
        .data = &reg_data,
        .len = 1,
    };
    reg_data = VCELL_4_2V | CSEL_3_CELL | IRCOMP_0MOHM;
    if (charge_write_nbyte(&data) != BSP_OK) {
        return BSP_ERROR;
    }
    /// 设置充电电流限制
    data.reg_addr = CHARGE_IBAT_CHG_REG;
    reg_data = BATT_INPUT_CURRENT_LIMIT / BATT_CURRENT_LIMIT_SCALE; /// 1000mA
    if (charge_write_nbyte(&data) != BSP_OK) {
        return BSP_ERROR;
    }
    /// 设置放电电流限制
    data.reg_addr = CHARGE_IBAT_DCHG_REG;
    reg_data = BATT_OUTPUT_CURRENT_LIMIT / BATT_CURRENT_LIMIT_SCALE; /// 3000mA
    if (charge_write_nbyte(&data) != BSP_OK) {
        return BSP_ERROR;
    }
    return BSP_OK;
}

static int charge_vin_config(void)
{
    uint8_t reg_data[2] = {0};
    i2c_master_data_t data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = CHARGE_VINDPM_H_REG,
        .data = reg_data,
        .len = 2,
    };
    /// 输入电压限制
    reg_data[0] = 0; 
    reg_data[1] = CHARGE_INPUT_VOLTAGE_LIMIT / CHARGE_INPUT_VOLTAGE_LIMIT_SCALE; /// 低于4.2V，启用限流
    if (charge_write_nbyte(&data) != BSP_OK) {
        return BSP_ERROR;
    }   
    /// 输入电流限制
    data.reg_addr = CHARGE_IIN_LIMIT_REG;
    data.len = 1;
    reg_data[0] = CHARGE_INPUT_CURRENT_LIMIT / CHARGE_INPUT_CURRENT_LIMIT_SCALE; /// 3000mA
    return charge_write_nbyte(&data);
}

static int charge_adc_config(uint8_t adc_en)
{
    uint8_t adc_set = 0;
    i2c_master_data_t data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = CHARGE_ADC_SET_REG,
        .data = &adc_set,
        .len = 1,
    };

    if (adc_en) {
        adc_set = AD_START_ENABLE | ADC_VBUS_ENABLE | ADC_IBUS_ENABLE |\
         ADC_VBAT_ENABLE | ADC_IBAT_ENABLE | ADC_NTC_ENABLE;
    }
    return charge_write_nbyte(&data);
}

static int charge_scale_config(void)
{
    uint8_t scale = 0;
    i2c_master_data_t data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = CHARGE_ADC_SCALE_REG,
        .data = &scale,
        .len = 1,
    };
    scale = VBUS_SCALE_CFG | IBUS_SCALE_CFG | VBAT_SCALE_CFG | IBAT_SCALE_CFG;
    return charge_write_nbyte(&data);
}

static int charge_interrupt_config(void)
{
    uint8_t reg_data[2] = {0};
    i2c_master_data_t data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = CHARGE_ADC_SCALE_REG,
        .data = reg_data,
        .len = 2,
    };
    reg_data[0] = CHARGE_AC_PRESENT_INT_ENABLE | CHARGE_INDET_INT_ENABLE | CHARGE_BUSUVP_CHG_INT_ENABLE | 
        CHARGE_BATOVP_INT_ENABLE | CHARGE_BAT_FULL_INT_ENABLE | CHARGE_RE_CHG_INT_ENABLE;
    reg_data[1] = CHARGE_CHG_TIMEOUT_INT_ENABLE | CHARGE_TSHUT_INT_ENABLE | CHARGE_BAT_EMPTY_INT_ENABLE | 
        CHARGE_NTC_FAULT_INT_ENABLE;
    return charge_write_nbyte(&data);
}

static int charge_status_poll(charge_chip_status_t *status)
{
    uint8_t reg_data[12] = {0};
    i2c_master_data_t data = {
        .dev_address = CHARGE_I2C_ADDR,
        .reg_addr = CHARGE_ADC_VBUS_L_REG,
        .data = (uint8_t *)&reg_data,
        .len = 12,
    };
    int ret = charge_read_nbyte(&data);
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
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
    charge_irq_control(true);
#endif
    LOG_PBUS("charge chip init success");
}

static void charge_device_thread(void *arg)
{
    uint8_t msg = 0;
    uint32_t log_timer = 0;
    uint16_t exit_delay_time = 30; /// 3000ms
    uint16_t plug_out_count = 0;  

    charge_chip_init();
    charge_status_poll(&s_charge_status);
    while (1) {
        MSG_QUEUE_RECV(s_charge_device_thd.msg_id, &msg, OS_WAIT_FOREVER);
        charge_status_poll(&s_charge_status);
        s_charge_status.charge_full = (s_charge_status.status & BAT_FULL_STATUS_POS) > 0 ? true : false;
        if (s_charge_status.vbus < CHARGE_INPUT_VOLTAGE_LIMIT) {
            if (s_charge_status.plug_in) {
                plug_out_count++;
                if (plug_out_count > 2) {     
                    s_charge_status.plug_in = false;
                    charge_power_control(false);
                    LOG_PBUS("charge plug out");
                }
            }
        } else {
            plug_out_count = 0;
            if (s_charge_status.plug_in == false) {
                s_charge_status.plug_in = true;
                charge_power_control(true);
                if (s_charge_irq_callback) {
                    s_charge_irq_callback(NULL);
                }
                LOG_PBUS("charge plug in");
            }
        }
        
        s_charge_status.temp = charge_ntc_temperature_get(s_charge_status.ntc_resistance);
        if (get_timestamp_ms(log_timer) > 2000) {
            log_timer = get_timestamp();
            BSP_LOG("charge status", "vbus:%d mV, ibus:%d mA, vbat:%d mV, ibat:%d mA, ntc:%f KΩ, temp:%d℃, status:%x", 
                     s_charge_status.vbus, s_charge_status.ibus, s_charge_status.vbat, s_charge_status.ibat, 
                     s_charge_status.ntc_resistance, s_charge_status.temp, s_charge_status.status);
        }

    #if (IS_SUPPORT_BATTERY_DIRECT_SAMPLING == 0)
        if (s_charge_battery_poll_callback) {
            s_charge_battery_poll_callback();
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
bool vbus_is_plug_in(void)
{
    return s_charge_status.plug_in;
}

void add_charge_irq_callback(void (*cb)(void *param))
{
    s_charge_irq_callback = cb;
}

#if (IS_SUPPORT_BATTERY_DIRECT_SAMPLING == 0)
void add_charge_battery_poll_callback(int (*cb)(void))
{
    s_charge_battery_poll_callback = cb;
}
#endif

int get_charge_state(void) 
{
    if (s_charge_is_ready == false) {
        return CHARGE_STATE_ERROR;
    }
    if (s_charge_status.charge_full && s_charge_status.plug_in) {
        return CHARGE_STATE_FULL;
    } else if (s_charge_status.plug_in) {
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
    int ret = BSP_OK;
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
    charge_irq_init();
#endif
    charge_power_gpio_init();
    bsp_i2c_init(CHARGE_I2C_ID);
#if IS_SUPPORT_PD_CHARGE
    bsp_charge_pd_init();
#endif
    s_charge_is_ready = true;
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
    START_TIMER(s_charge_timer_id, CHARGE_THREAD_POLL_INTERVAL);
#if IS_SUPPORT_PD_CHARGE
    bsp_charge_pd_resume();
#endif
    return BSP_OK;
}

int bsp_charge_suspend(void)
{
    s_charge_is_ready = false;
    s_charge_abnormal_count = 0;
    STOP_TIMER(s_charge_timer_id);
#if IS_SUPPORT_PD_CHARGE
    bsp_charge_pd_suspend();
#endif
    return BSP_OK;
}

int bsp_charge_is_ready(void *args)
{
    uint8_t *charge_is_ready_status = (uint8_t *)args;
    *charge_is_ready_status = s_charge_is_ready;
    return BSP_OK;
}
