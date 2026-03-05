/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_config.h"
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_internal.h" // IWYU pragma: keep
#include "bsp_i2c.h" // IWYU pragma: keep
#include "bsp_sc09b.h" // IWYU pragma: keep
#include "bsp_key_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define TOUCH_KEY_NUM               6
#define TOUCH_KEY_RETRY_COUNT       1
#define TOUCH_KEY_IDLE_COUNT        3
#define TOUCH_I2C_ADDR              0x40  /// SC09B 只有一个固定地址

#define TOUCH_ABNORMAL_MAX_COUNT    10
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t s_touch_abnormal_count = 0;
static bool s_touch_is_ready = false;
static thread_context_t s_key_device_thd  = {0};
static uint8_t s_touch_key[TOUCH_KEY_NUM] = {0};
static uint16_t s_key_mask[TOUCH_KEY_NUM] = {
#if V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1
    0x0002, /// 启动键
    0x0004, /// 模式键
    0x0008, /// 频段键
    0x0010, /// 追奶键
    0x0001, /// 左按键
    0x4000, /// 右按键
    // 0x8000, /// 双边控制键
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3)
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3)
    0x0010, /// 启动键//
    0x8000, /// 模式键
    0x0002, /// 频段键
    0x0001, /// 追奶键
    0x0008, /// 左按键//
    0x0004, /// 右按键
#else
    0x0001, /// 启动键
    0x0010, /// 模式键
    0x0008, /// 频段键
    0x0004, /// 追奶键
    0x8000, /// 左按键
    0x0002, /// 右按键
#endif
#else
    0x0002, /// 启动键
    0x0004, /// 模式键
    0x0008, /// 频段键
    0x0010, /// 追奶键
    0x0001, /// 左按键
    0x4000, /// 右按键
#endif
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
POSSIBLY_UNUSED static int sc09b_write_nbyte(i2c_master_data_t *data)
{
    int ret = 0;

    if (s_touch_is_ready == 0) {
        LOGE("touch key is not ready");
        return BSP_NOT_READY;
    }

    for (uint8_t i = 0; i < TOUCH_KEY_RETRY_COUNT; i++) {
        ret = bsp_i2c_write_byte(TOUCH_KEY_I2C_ID, data);
        if (ret == BSP_OK) {
            s_touch_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_touch_abnormal_count++;
    if (s_touch_abnormal_count > TOUCH_ABNORMAL_MAX_COUNT) {
        s_touch_is_ready = 0;
    }

    return BSP_ERROR;
}

POSSIBLY_UNUSED static int sc09b_read_nbyte(i2c_master_data_t *data)
{
    int ret = 0;

    if (s_touch_is_ready == 0) {
        LOGE("touch key is not ready");
        return BSP_NOT_READY;
    }

    for (uint8_t i = 0; i < TOUCH_KEY_RETRY_COUNT; i++) {
        ret = bsp_i2c_read_byte(TOUCH_KEY_I2C_ID, data);
        if (ret == BSP_OK) {
            s_touch_abnormal_count = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_touch_abnormal_count++;
    if (s_touch_abnormal_count > TOUCH_ABNORMAL_MAX_COUNT) {
        s_touch_is_ready = 0;
    }

    return BSP_ERROR;
}

POSSIBLY_UNUSED static void sc09b_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    uint8_t msg = 0;
    // LOG_PBUS("sc09b pin[%d] irq triggered", pin);
    if (pin == TOUCH_INT_PIN && s_key_device_thd.msg_id) {
        MSG_QUEUE_SEND(s_key_device_thd.msg_id, &msg, 0);
        bsp_irq_msg_send(INT_NUM_TOUCH_KEY);
    }
}

POSSIBLY_UNUSED static void sc09b_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable, 
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, 
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING, 
        sc09b_irq_callback
    };
    hal_gpio_setup_irq(TOUCH_INT_PIN, &gpio_irq_cfg);
}

POSSIBLY_UNUSED static void sc09b_irq_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)TOUCH_INT_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(TOUCH_INT_PIN,  HAL_GPIO_DIR_IN,  1);
}

POSSIBLY_UNUSED static void sc09b_touch_key_scan(void)
{
    uint16_t key_value[2] = {0}, key_idle_count = 0, error_count = 0;
    i2c_master_data_t key_data = {
        .dev_address = TOUCH_I2C_ADDR,
        .data = (uint8_t *)&key_value,
        .len  = 2,
        .reg_addr = SC09B_OUTPUT1_REG
    };

    while(1) {
        if (sc09b_read_nbyte(&key_data) == BSP_OK) {
            for (uint8_t i = 0; i < TOUCH_KEY_NUM; i++) {
                s_touch_key[i] = (key_value[0] & s_key_mask[i]) ? 1 : 0;
            }
            // LOG_PBUS("key value: %x, %x, %x, %x, %x, %x", s_touch_key[0], s_touch_key[1], 
            //     s_touch_key[2], s_touch_key[3], s_touch_key[4], s_touch_key[5]);
            // BSP_LOG_PASS("key value: %x, %x\r\n", key_value[0], key_value[1]);
            if (key_value[0] == 0) {
                key_idle_count++;
                if (key_idle_count > TOUCH_KEY_IDLE_COUNT) {
                    break;
                }
            } else {
                key_idle_count = 0;
            }
            error_count = 0;
        } else {
            error_count++;
            if (error_count > 5) {
                break;
            }
        }
        msleep(10);
    }
}

POSSIBLY_UNUSED static void key_device_thread(void *arg)
{
    int ret = 0;
    uint8_t msg = 0;
    uint32_t timestamp = 0;
    while (1) {
        MSG_QUEUE_RECV(s_key_device_thd.msg_id, &msg, OS_WAIT_FOREVER);
        timestamp = get_timestamp();
        sc09b_touch_key_scan();
        // LOG_PBUS("key release time:", get_timestamp_ms(timestamp));
    }
}

POSSIBLY_UNUSED static void sc09b_reference_cap_config(void)
{
    uint8_t reg_val = SC09B_CAP_DEFALUT;
    i2c_master_data_t key_data = {
        .dev_address = TOUCH_I2C_ADDR,
        .data = &reg_val,
        .len  = 1,
        .reg_addr = SC09B_OUTPUT1_REG 
    };
    sc09b_write_nbyte(&key_data);
}

POSSIBLY_UNUSED static int sc09b_is_ready(void)
{
    int ret = BSP_NOT_READY;
    uint16_t reg_val = 0;
    i2c_master_data_t key_data = {
        .dev_address = TOUCH_I2C_ADDR,
        .data = (uint8_t *)&reg_val,
        .len  = 2,
        .reg_addr = SC09B_OUTPUT1_REG 
    };

    for (uint8_t i = 0; i < 5; i++) {
        ret = bsp_i2c_read_byte(TOUCH_KEY_I2C_ID, &key_data);
        if (ret != BSP_OK) {
            continue;
        }
        msleep(5);
    }

    return ret;
}

POSSIBLY_UNUSED static int sc09b_chip_init(void)
{
    // bsp_key_sensitivity_set(0, SC09B_SEN_6);
    // bsp_key_sensitivity_set(1, SC09B_SEN_6);
    // sc09b_reference_cap_config();
    return BSP_OK;
}


static void power_key_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    // LOG_PBUS("power key pin[%d] irq triggered", pin);
    bsp_irq_msg_send(INT_NUM_KEY_1);
}

static void power_key_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable, 
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, 
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING, 
        power_key_irq_callback
    };

    hal_gpio_setup_irq(POWER_KEY_PIN, &gpio_irq_cfg);
}

static void power_key_irq_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)POWER_KEY_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(POWER_KEY_PIN,  HAL_GPIO_DIR_IN,  1);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_key_init(void)
{
    /// 初始化电源键
    power_key_irq_init();
    power_key_irq_control(true);
#if IS_SUPPORT_DEVICE_TOUCH_KEY
    /// 初始化触摸键
    sc09b_irq_init();
    bsp_i2c_init(TOUCH_KEY_I2C_ID);
    if (sc09b_is_ready() != BSP_OK) {
        LOGE("touch key is not ready");
        return BSP_NOT_READY;
    }
    s_touch_is_ready = true;
    s_touch_abnormal_count = 0;
    if (sc09b_chip_init() != BSP_OK) {
       LOGE("touch key init failed");
        return BSP_ERROR;
    }
    CREATE_MSG_TYPE_QUEUE(&s_key_device_thd.msg_id, 1, 1);
    RUN_NEW_THREAD(&s_key_device_thd, "key", key_device_thread, RT_PRIO_KEY_DEVICE_THREAD, (KEY_DEVICE_THREAD_STACKSIZE));
    sc09b_irq_control(true);
    LOG_PBUS("key init success");
#endif
    return BSP_OK;
}

int bsp_key_deinit(void)
{
#if IS_SUPPORT_DEVICE_TOUCH_KEY
    sc09b_irq_control(false);
#endif
    bsp_i2c_deinit(TOUCH_KEY_I2C_ID);
    s_touch_is_ready = false;
    s_touch_abnormal_count = 0;
    return BSP_OK;
}

int bsp_read_key_value(uint8_t key_index)
{
    if (key_index > TOUCH_KEY_NUM) {
        return 0;
    }

    if (key_index == 0) {
        return hal_gpio_pin_get_val(POWER_KEY_PIN) == 0 ? 1 : 0;
    }
    return s_touch_key[key_index - 1];
}

int bsp_read_all_key_value(void)
{
    int key_value = 0;
    for (uint8_t i = 1; i <= TOUCH_KEY_NUM; i++) {
        key_value |= s_touch_key[TOUCH_KEY_NUM - i];
        key_value <<= 1;
    }
    key_value |= hal_gpio_pin_get_val(POWER_KEY_PIN) == 0 ? 1 : 0;
    return key_value;
}

int bsp_key_sensitivity_set(uint8_t key_index, uint8_t sensitivity)
{
#if IS_SUPPORT_DEVICE_TOUCH_KEY
    uint8_t reg_val = sensitivity;
    i2c_master_data_t key_data = {
        .dev_address = TOUCH_I2C_ADDR,
        .data = &reg_val,
        .len  = 2,
        .reg_addr = key_index == 0 ? SC09B_SEN_REG0 : SC09B_SEN_REG_COM 
    };

    return sc09b_write_nbyte(&key_data);
#else
    return BSP_OK;
#endif
}

int bsp_touch_is_ready(void *args)
{
    uint8_t *touch_is_ready_status = (uint8_t *)args;
    *touch_is_ready_status = s_touch_is_ready;
    return BSP_OK;
}