/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "hal_iomux.h"
#include "hal_i2c.h"
#include "hal_timer.h"
#include "bsp_common.h"
#include "bsp_config.h"
#include "bsp_i2c.h"

/*
 * DEFINES
 *****************************************************************************************
 */
#define I2C_MASTER_TIMEOUT_MS(x)                    ((x > 20) ? ((x / 20) * 50) : 50)

#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
#define I2C_MASTER_COUNT                            4
#define I2C0_MASTER_SPEED                           400000
#define I2C1_MASTER_SPEED                           200000
#define I2C2_MASTER_SPEED                           100000
#define I2C3_MASTER_SPEED                           400000
#define IS_SUPPORT_I2C_MASTER_TASK                  1 
#else
#define I2C_MASTER_COUNT                            3
#define I2C_MASTER_SPEED                            400000
#define IS_SUPPORT_I2C_MASTER_TASK                  0
#endif

#define I2C_MASTER_ID_IS_INVALID(x) \
do { \
    if ((x >= I2C_MASTER_ID_MAX) || (x < 0)) { \
        LOGW("device_id out of range:%d", x); \
        return BSP_ERROR; \
    } \
} while (0)

typedef struct {
    enum HAL_I2C_ID_T id;
    void (*iomux_set)(void);
    enum HAL_GPIO_PIN_T scl_pin;
    enum HAL_GPIO_PIN_T sda_pin;
    uint32_t speed;
} i2c_master_config_t;

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static bool s_i2c_is_ready[I2C_MASTER_COUNT] = {false};
static uint8_t s_i2c_init_count[I2C_MASTER_COUNT] = {0};
static os_mutex_id s_i2c_mux[I2C_MASTER_COUNT] = {NULL};

static const i2c_master_config_t s_i2c_config[I2C_MASTER_COUNT] = {
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)    
    {
        .id = HAL_I2C_ID_0, 
        .iomux_set = hal_iomux_set_i2c0,
        .scl_pin = I2C0_SCL_PIN,
        .sda_pin = I2C0_SDA_PIN,
        .speed = I2C1_MASTER_SPEED,
    },
    {
        .id = HAL_I2C_ID_1, 
        .iomux_set = hal_iomux_set_i2c1,
        .scl_pin = I2C1_SCL_PIN,
        .sda_pin = I2C1_SDA_PIN,
        .speed = I2C1_MASTER_SPEED,
    },
    {
        .id = HAL_I2C_ID_2, 
        .iomux_set = hal_iomux_set_i2c2,
        .scl_pin = I2C2_SCL_PIN,
        .sda_pin = I2C2_SDA_PIN,
        .speed = I2C2_MASTER_SPEED,
    },
    {
        .id = HAL_I2C_ID_3, 
        .iomux_set = hal_iomux_set_i2c3,
        .scl_pin = I2C3_SCL_PIN,
        .sda_pin = I2C3_SDA_PIN,
        .speed = I2C3_MASTER_SPEED,
    }
#else
    {
        .id = HAL_I2C_ID_2, 
        .iomux_set = hal_iomux_set_i2c2,
        .scl_pin = I2C2_SCL_PIN,
        .sda_pin = I2C2_SDA_PIN,
        .speed = I2C_MASTER_SPEED,
    },
    {
        .id = HAL_I2C_ID_3, 
        .iomux_set = hal_iomux_set_i2c3,
        .scl_pin = I2C3_SCL_PIN,
        .sda_pin = I2C3_SDA_PIN,
        .speed = I2C_MASTER_SPEED,
    },
    {
        .id = HAL_I2C_ID_4, 
        .iomux_set = hal_iomux_set_i2c4,
        .scl_pin = I2C4_SCL_PIN,
        .sda_pin = I2C4_SDA_PIN,
        .speed = I2C_MASTER_SPEED,
    }
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

static void bsp_i2c_abnormal_recovery(const i2c_master_config_t *config)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)config->scl_pin,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(config->scl_pin, HAL_GPIO_DIR_OUT, 1);
    gpio_map.pin = (enum HAL_IOMUX_PIN_T)config->sda_pin;
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(config->sda_pin, HAL_GPIO_DIR_IN, 0);
    if (hal_gpio_pin_get_val(config->sda_pin)) {
        return;
    }
    LOGW("i2c %d is abnormal", config->id);
    /// 发送9个时钟信号
    for (int i = 0; i < 9; i++) {
        GPIO_PIN_CTRL(config->scl_pin, 1);
        hal_sys_timer_delay_us(5);
        GPIO_PIN_CTRL(config->scl_pin, 0);
        hal_sys_timer_delay_us(5);
    }
    /// 检查SDA是否为低电平
    if (hal_gpio_pin_get_val(config->sda_pin) == 0) {
        LOGE("i2c %d abnormal recovery failed, sda_pin: %d, scl_pin: %d",
             config->id, config->sda_pin, config->scl_pin);
        return;
    }
    /// 发送停止信号
    GPIO_PIN_CTRL(config->scl_pin, 1);
    hal_sys_timer_delay_us(5);
    hal_gpio_pin_set_dir(config->sda_pin, HAL_GPIO_DIR_OUT, 1);
    LOGW("i2c %d abnormal recovery success, sda_pin: %d, scl_pin: %d",
         config->id, config->sda_pin, config->scl_pin);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_i2c_write_byte(i2c_master_id_e device_id, i2c_master_data_t *data)
{
    int ret = 0;
    I2C_MASTER_ID_IS_INVALID(device_id);
    EXCEPTION_CHECK_INT(data);
    uint16_t data_len = data->len + data->reg_addr_len;
    if (data_len > 1024) {
        LOGE("data_len is too long: %d", data_len);
        return BSP_ERROR;
    }
    uint8_t * data_buf = (uint8_t *)u_malloz(data_len < 128 ? 128 : data_len);
    if (!data_buf) {
        return BSP_ERROR;
    }

    if (data->reg_addr_len > 0) {
        memcpy(data_buf, data->reg_addr, data->reg_addr_len);
    }

    if (data->len > 0) {
        memcpy(&data_buf[data->reg_addr_len], data->data, data->len);
    }
    MUTEX_LOCK(s_i2c_mux[device_id]);
    if (s_i2c_is_ready[device_id]) {
        #if IS_SUPPORT_I2C_MASTER_TASK
        ret = (int)hal_i2c_task_send(s_i2c_config[device_id].id, data->dev_address, data_buf, data_len, 0, NULL);
        #else
        ret = (int)hal_i2c_simple_send(s_i2c_config[device_id].id, data->dev_address, 
            data_buf, data->len + 1);
        #endif
        if (ret != 0) {
            LOGE("write i2c-%d timeout:%x, ret:%d", device_id, data->dev_address, ret);
            BSP_LOG_HEX("i2c_write_byte", data_buf, data_len);
        }
    } else {
        LOGE("i2c is no ready"); 
    }
    MUTEX_UNLOCK(s_i2c_mux[device_id]);
    u_free(data_buf);

    return ret;
}

int bsp_i2c_read_byte(i2c_master_id_e device_id, i2c_master_data_t *data)
{
    int ret = 0;
    I2C_MASTER_ID_IS_INVALID(device_id);
    EXCEPTION_CHECK_INT(data);
    MUTEX_LOCK(s_i2c_mux[device_id]);
    if (s_i2c_is_ready[device_id]) {
        #if IS_SUPPORT_I2C_MASTER_TASK
        ret = (int)hal_i2c_task_recv(s_i2c_config[device_id].id, data->dev_address, data->reg_addr,
            data->reg_addr_len, data->data, data->len, 0, NULL);
        #else
        ret = (int)hal_i2c_simple_recv(s_i2c_config[device_id].id, data->dev_address, data->reg_addr,
            data->reg_addr_len, data->data, data->len);
        #endif
        if (ret != 0) {
            LOGE("read i2c-%d timeout:%x ret:%d", device_id, data->dev_address, ret);
            BSP_LOG_HEX("i2c_read_byte", data->data, data->len);
        }
    } else {
        LOGE("i2c is no ready");  
    }
    MUTEX_UNLOCK(s_i2c_mux[device_id]);
  
    return ret;
}

int bsp_i2c_init(i2c_master_id_e device_id)
{
    int ret = 0;
    I2C_MASTER_ID_IS_INVALID(device_id);
    struct HAL_I2C_CONFIG_T i2c_cfg = {
    #if IS_SUPPORT_I2C_MASTER_TASK
        .mode = HAL_I2C_API_MODE_TASK,
        .use_dma = false, // 当前SDK只有I2C0和I2C1支持dma
        .use_sync = true,
    #else
        .mode = HAL_I2C_API_MODE_SIMPLE,
    #endif
        .as_master = true,
        .rising_time_ns = 0,
        .speed = s_i2c_config[device_id].speed,
        .addr_as_slave = 0,
    };
    s_i2c_init_count[device_id]++;
    if (s_i2c_is_ready[device_id]) {
        LOG_PBUS("i2c reinit:%d", s_i2c_init_count[device_id]);
        return BSP_OK;
    }
    bsp_i2c_abnormal_recovery(&s_i2c_config[device_id]);    /// 检测I2C是否异常
    if (s_i2c_mux[device_id] == NULL) {
        MUTEX_INIT(&s_i2c_mux[device_id], dev_i2c_mutex);
    }
    MUTEX_LOCK(s_i2c_mux[device_id]);
    s_i2c_config[device_id].iomux_set();
    ret = (int)hal_i2c_open(s_i2c_config[device_id].id, &i2c_cfg);
    if (ret != BSP_OK) {
        LOGE("i2c %d open failed: %x, scl_pin: %d, sda_pin: %d", device_id, ret, s_i2c_config[device_id].scl_pin, s_i2c_config[device_id].sda_pin);
        goto ERROR_EXIT;
    }
    s_i2c_is_ready[device_id] = true;
    LOG_PBUS("i2c %d open success", device_id);

ERROR_EXIT:
    MUTEX_UNLOCK(s_i2c_mux[device_id]);
    return BSP_OK;
}

int bsp_i2c_deinit(i2c_master_id_e device_id)
{
    int ret = 0;
    I2C_MASTER_ID_IS_INVALID(device_id);
    MUTEX_LOCK(s_i2c_mux[device_id]);
    if (s_i2c_init_count[device_id] > 0) {
        s_i2c_init_count[device_id]--;
        if (s_i2c_init_count[device_id] > 0) {
            LOGE("i2c deinit is busy:%d", s_i2c_init_count[device_id]);
            MUTEX_UNLOCK(s_i2c_mux[device_id]);
            return BSP_OK;
        }
    }
    s_i2c_is_ready[device_id] = false;
    ret = (int)hal_i2c_close(s_i2c_config[device_id].id);
    MUTEX_UNLOCK(s_i2c_mux[device_id]);
    LOG_PBUS("i2c_driver_delete:%d, ret:%d", device_id, ret);
    return ret;
}
