/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "hal_i2c.h" // IWYU pragma: keep
#include "hal_iomux.h" // IWYU pragma: keep
#include "hal_gpio.h" // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_internal.h" // IWYU pragma: keep
#include "bsp_i2c.h" // IWYU pragma: keep
#include "bsp_sc09b.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define TOUCH_INT_PIN            HAL_IOMUX_PIN_P0_0
#define TOUCH_KEY_NUM            6

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static thread_context_t s_key_device_thd  = {0};
static uint8_t s_touch_key[TOUCH_KEY_NUM] = {0};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void sc09b_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    uint8_t msg = 0;
    LOG_PBUS("sc09b pin[%d] irq triggered", pin);
    if (pin == (enum HAL_GPIO_PIN_T)TOUCH_INT_PIN && s_key_device_thd.msg_id) {
        MSG_QUEUE_SEND(s_key_device_thd.msg_id, &msg, 0);
        bsp_irq_msg_send(INT_NUM_TOUCH_KEY);
    }
}

static void sc09b_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable, 
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, 
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING, 
        sc09b_irq_callback
    };
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)TOUCH_INT_PIN, &gpio_irq_cfg);
}

static void sc09b_irq_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        TOUCH_INT_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)TOUCH_INT_PIN,  HAL_GPIO_DIR_IN,  1);
}

static void sc09b_touch_key_scan(void)
{
    uint8_t touch_key_idle = 0;
    while (1) {
        // TODO: scan touch key
        memset(s_touch_key, 0, sizeof(s_touch_key)); // 测试
        if (touch_key_idle == 0) {
            break;
        }
        msleep(500);
    }
}

static void key_device_thread(void *arg)
{
    int ret = 0;
    uint8_t buf[10] = {0};
    i2c_master_data_t data = {
        .dev_address = 0x10,
        .reg_addr = 0x00,
        .data = buf,
        .len = 10,
    };
    uint32_t timestamp = 0, offset_time = 0;
    while (1) {
        // timestamp = get_timestamp();
        // ret = bsp_i2c_read_byte(I2C_MASTER_ID_0, &data);
        // offset_time = get_timestamp() - timestamp;
        // LOG_PBUS("key device thread ret=%d, buf[0]=%d, offset_time=%d", ret, buf[0], offset_time);
        msleep(500);
    }
}

static void sc09b_chip_init(void)
{


}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_key_init(void)
{
    sc09b_irq_init();
    bsp_i2c_init(I2C_MASTER_ID_0);
    CREATE_MSG_TYPE_QUEUE(&s_key_device_thd.msg_id, 1, 1);
    RUN_NEW_THREAD(&s_key_device_thd, "key", key_device_thread, RT_PRIO_KEY_DEVICE_THREAD, (KEY_DEVICE_THREAD_STACKSIZE));
    sc09b_irq_control(true);
    sc09b_chip_init();
    return BSP_OK;
}

int bsp_key_deinit(void)
{
    sc09b_irq_control(false);
    bsp_i2c_deinit(I2C_MASTER_ID_0);
    return BSP_OK;
}

int bsp_read_key_value(uint8_t key_index)
{
    if (key_index >= TOUCH_KEY_NUM) {
        return 0;
    }
    return s_touch_key[key_index];
}

int bsp_key_sensitivity_set(uint8_t key_index, uint8_t sensitivity)
{
    return BSP_OK;
}