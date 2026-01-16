/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "bsp_charge_pd.h" 

/*
* DEFINES
*****************************************************************************************
*/
#define CHARGE_PD_I2C_ADDR                     0x23
#define CHARGE_PD_RETRY_COUNT                  3
#define CHARGE_PD_ABNORMAL_MAX_COUNT           10
/*
* LOCAL VARIABLE DEFINITIONS)
*****************************************************************************************
*/
static uint16_t s_charge_pd_abnormal_count = 0;
static bool s_charge_pd_is_ready = false;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int charge_pd_write_nbyte(i2c_master_data_t *data)
{
    int ret = 0;

    if (s_charge_pd_is_ready == 0) {
        LOGE("charge is not ready");
        return BSP_NOT_READY;
    }

    for (uint8_t i = 0; i < CHARGE_PD_RETRY_COUNT; i++) {
        ret = bsp_i2c_write_byte(CHARGE_PD_I2C_ID, data);
        if (ret == BSP_OK) {
            s_charge_pd_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_charge_pd_abnormal_count++;
    if (s_charge_pd_abnormal_count > CHARGE_PD_ABNORMAL_MAX_COUNT) {
        s_charge_pd_is_ready = 0;
    }

    return BSP_ERROR;
}

static int charge_pd_read_nbyte(i2c_master_data_t *data)
{
    int ret = 0;
    
    if (s_charge_pd_is_ready == 0) {
        LOGE("charge is not ready");
        return BSP_NOT_READY;
    }

    for (uint8_t i = 0; i < CHARGE_PD_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(CHARGE_I2C_ID, data);
        if (ret == BSP_OK) {
            s_charge_pd_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_charge_pd_abnormal_count++;
    if (s_charge_pd_abnormal_count > CHARGE_PD_ABNORMAL_MAX_COUNT) {
        s_charge_pd_is_ready = 0;
    }

    return BSP_ERROR;
}

static void charge_pd_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    bsp_irq_msg_send(INT_NUM_CHARGE_PD);
}

static void charge_pd_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable, 
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, 
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING, 
        charge_pd_irq_callback
    };

    hal_gpio_setup_irq(CHARGE_PD_INT_PIN, &gpio_irq_cfg);
}

static void charge_pd_irq_init(void)
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

static int charge_pd_device_is_ready(void)
{
    int ret = 0;
    uint8_t value = 0;
    i2c_master_data_t read_data = {
        .dev_address = CHARGE_PD_I2C_ADDR,
        .reg_addr = CHARGE_PD_STATUS_REG,
        .data = &value,
        .len = 1,
    };

    for (uint8_t i = 0; i < 3; i++) {
        ret = bsp_i2c_read_byte(CHARGE_PD_I2C_ID, &read_data);
        if (ret == BSP_OK) {
            break;
        }
        msleep(10);
    }

    return ret;
}

static int charge_pd_voltage_config(charge_pd_voltage_e voltage)
{
    uint8_t reg_data = (uint8_t)voltage;
    i2c_master_data_t data = {
        .dev_address = CHARGE_PD_I2C_ADDR,
        .reg_addr = CHARGE_PD_VOLTAGE_REG,
        .data = &reg_data,
        .len = 1,
    };
    reg_data = voltage;
    return charge_pd_write_nbyte(&data);
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int bsp_charge_pd_power_data_get(uint8_t *data, uint8_t len)
{
    i2c_master_data_t read_data = {
        .dev_address = CHARGE_PD_I2C_ADDR,
        .reg_addr = CHARGE_PD_POWER_DATA_REG,
        .data = data,
        .len = len,
    };

    if (charge_pd_read_nbyte(&read_data) != BSP_OK) {
        return BSP_ERROR;
    }
    BSP_LOG_HEX("charge_pd_power_data", data, len);
    
    return BSP_OK;
}
 
int bsp_charge_pd_init(void)
{
    charge_pd_irq_init();
    charge_pd_irq_control(true);
    bsp_i2c_init(CHARGE_PD_I2C_ID);
    s_charge_pd_is_ready = true;
    s_charge_pd_abnormal_count = 0;
    return BSP_OK;
}

int bsp_charge_pd_resume(void)
{
    s_charge_pd_is_ready = true;
    s_charge_pd_abnormal_count = 0;
    return BSP_OK;
}

int bsp_charge_pd_suspend(void)
{
    s_charge_pd_is_ready = false;
    s_charge_pd_abnormal_count = 0;
    return BSP_OK;
}

int bsp_charge_pd_is_ready(void *args)
{
    uint8_t *pd_is_ready_status = (uint8_t *)args;
    *pd_is_ready_status = s_charge_pd_is_ready;
    return BSP_OK;
}

void charge_pd_event_handler(void)
{
    if (hal_gpio_pin_get_val(CHARGE_PD_INT_PIN) != 0) {
        LOGE("charge pd power off");
        return;
    } 

    if (charge_pd_device_is_ready() != BSP_OK) {
        return;
    }
    
    LOG_PBUS("charge pd power on");
}
