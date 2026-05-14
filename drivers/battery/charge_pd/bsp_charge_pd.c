/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "bsp_charge_pd.h" 
#include "bsp_battery_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define CHARGE_PD_I2C_ADDR                     0x23
#define CHARGE_PD_RETRY_COUNT                  3
#define CHARGE_PD_ABNORMAL_MAX_COUNT           10
#define CHARGE_PD_VERSION_1                    0    /// PD1.0
#define CHARGE_PD_VERSION_2                    1    /// PD2.0
#define CHARGE_PD_VERSION_3                    2    /// PD3.0

typedef enum {
    CHARGE_PD_ADAPTER_PLUG_OUT = 0, /// 适配器拔出
    CHARGE_PD_ADAPTER_READY = 1, /// 适配器准备就绪
    CHARGE_PD_ADAPTER_MISMATCH = 2, /// 适配器不匹配
} charge_pd_adapter_status_e;

/*
* LOCAL VARIABLE DEFINITIONS)
*****************************************************************************************
*/
static bool s_charge_pd_onoff = false;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int charge_pd_write_nbyte(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;

    i2c_master_data_t i2c_data = {
        .dev_address = CHARGE_PD_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };
    
    for (uint8_t i = 0; i < CHARGE_PD_RETRY_COUNT; i++) {
        ret = bsp_i2c_write_byte(CHARGE_PD_I2C_ID, &i2c_data);
        if (ret == BSP_OK) {
            return BSP_OK;
        }
        msleep(10);
    }
    return BSP_ERROR;
}

static int charge_pd_read_nbyte(uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;

    i2c_master_data_t i2c_data = {
        .dev_address = CHARGE_PD_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    for (uint8_t i = 0; i < CHARGE_PD_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(CHARGE_PD_I2C_ID, &i2c_data);
        if (ret == BSP_OK) {
            return BSP_OK;
        }
        msleep(10);
    }
    return BSP_ERROR;
}

static int charge_pd_device_is_ready(void)
{
    uint8_t status = 0;
    if (charge_pd_read_nbyte(CHARGE_PD_STATUS_REG, &status, 1) != BSP_OK) {
        return BSP_ERROR;
    }

    if (status != 0) {
        return BSP_OK;
    }

    return BSP_ERROR;
}

static int charge_pd_voltage_config(charge_pd_voltage_e voltage)
{
    uint8_t reg_data = (uint8_t)voltage;
    reg_data = voltage;
    return charge_pd_write_nbyte(CHARGE_PD_VOLTAGE_REG, &reg_data, 1);
}

static int charge_pd_version_get(void)
{
    uint8_t pd_header = 0;
    uint8_t version = 0;
    if (charge_pd_read_nbyte(CHARGE_PD_POWER_DATA_REG, &pd_header, 1) != BSP_OK) {
        return -1;
    }
    version = (pd_header >> 6) & 0x03;
    // LOG_PBUS("version: %d", version);
    return version;
}

static bool charge_pd_power_is_ready(void)
{
    if (charge_pd_device_is_ready() != BSP_OK) {
        // LOG_PBUS("charge pd device not ready");
        return false;
    }

    int version = charge_pd_version_get();
    if (version < CHARGE_PD_VERSION_2) {
        // LOG_PBUS("charge pd version too low, version: %d", version);
        return false;
    }
    return true;
}

static void charge_pd_gpio_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)CHARGE_PD_I2C_CLK_PIN, 
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(CHARGE_PD_I2C_CLK_PIN, HAL_GPIO_DIR_IN, 0);
    gpio_map.pin = (enum HAL_IOMUX_PIN_T)CHARGE_PD_I2C_DATA_PIN;
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(CHARGE_PD_I2C_DATA_PIN, HAL_GPIO_DIR_IN, 0);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int bsp_charge_pd_is_ready(void)
{
    if (s_charge_pd_onoff == false) {
        return false;
    }
    return charge_pd_device_is_ready();
}

void charge_pd_status_poll(bool plug_in_debounce)
{
    static uint8_t debounce_count = 0;
    if (hal_gpio_pin_get_val(CHARGE_PD_INT_PIN) == 0) {
        if (s_charge_pd_onoff == false) {
            bsp_i2c_init(CHARGE_PD_I2C_ID);
            s_charge_pd_onoff = true;
            debounce_count = 0;
            LOG_PBUS("%s", "charge pd plug in");
            return;
        }

        if (charge_pd_power_is_ready()) {
            charge_event_internal_cb(CHARGE_EVENT_PLUG_IN);
        } else {
            if (debounce_count < CHARGE_PD_ABNORMAL_MAX_COUNT && plug_in_debounce) {
                debounce_count++;
            } else{
                charge_event_internal_cb(CHARGE_EVENT_ADAPTER_MISMATCH);
            }
        }
    } else {
        if (s_charge_pd_onoff == true) {
            bsp_i2c_deinit(CHARGE_PD_I2C_ID);
            charge_event_internal_cb(CHARGE_EVENT_PLUG_OUT);
            charge_pd_gpio_init();
            s_charge_pd_onoff = false;
            LOG_PBUS("%s", "charge pd plug out");
        }
    }
}

void bsp_charge_pd_init(void)
{
    charge_pd_gpio_init();
    msleep(10);
    for (uint8_t i = 0; i < 3; i++) {
        charge_pd_status_poll(false); /// 插入开机无需去抖
        msleep(10);
    }
}

int charge_pd_current_get(void)
{
    uint8_t current = 0;
    if (charge_pd_read_nbyte(CHARGE_PD_OUTPUT_CURRENT_REG, &current, 1) != BSP_OK) {
        return 0;
    }
    return current * 50; /// 50mA
}
