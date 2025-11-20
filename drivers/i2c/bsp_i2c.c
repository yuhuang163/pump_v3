/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "hal_iomux.h"
#include "hal_i2c.h"
#include "bsp_common.h"
#include "root_log_define.h"
#include "bsp_i2c.h"

/*
 * DEFINES
 *****************************************************************************************
 */
#define I2C_MASTER_TIMEOUT_MS(x)                    ((x > 20) ? ((x / 20) * 50) : 50)
#define I2C_MASTER_COUNT                            3
#define I2C_MASTER_SPEED                            400000
#define IS_SUPPORT_I2C_MASTER_TASK                  0 
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
} i2c_master_config_t;

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static bool s_i2c_is_ready[I2C_MASTER_COUNT] = {false};
static uint8_t s_i2c_init_count[I2C_MASTER_COUNT] = {0};
static os_mutex_id s_i2c_mux[I2C_MASTER_COUNT] = {NULL};
static const i2c_master_config_t s_i2c_config[I2C_MASTER_COUNT] = {
    {
        .id = HAL_I2C_ID_2, 
        .iomux_set = hal_iomux_set_i2c2,
    },
    {
        .id = HAL_I2C_ID_3, 
        .iomux_set = hal_iomux_set_i2c3,
    },
    {
        .id = HAL_I2C_ID_4, 
        .iomux_set = hal_iomux_set_i2c4,
    }
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int bsp_i2c_write_byte(i2c_master_id_e device_id, i2c_master_data_t *data)
{
    int ret = 0;
    I2C_MASTER_ID_IS_INVALID(device_id);
    EXCEPTION_CHECK_INT(data);
    uint8_t * data_buf = (uint8_t *)u_malloz(data->len + 1);
    if (!data_buf) {
        return BSP_ERROR;
    }
    data_buf[0] = data->reg_addr;
    memcpy(&data_buf[1], data->data, data->len);
    MUTEX_LOCK(s_i2c_mux[device_id]);
    if (s_i2c_is_ready[device_id]) {
        #if IS_SUPPORT_I2C_MASTER_TASK
        ret = (int)hal_i2c_task_send(s_i2c_config[device_id].id, data->dev_address, data_buf, data->len + 1, 0, NULL);
        #else
        ret = (int)hal_i2c_simple_send(s_i2c_config[device_id].id, data->dev_address, 
            data_buf, data->len + 1);
        #endif
        if (ret != 0) {
            LOGE("write i2c-%d timeout:%x, ret:%d", device_id, data->dev_address, ret);
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
        ret = (int)hal_i2c_task_recv(s_i2c_config[device_id].id, data->dev_address, &data->reg_addr,
            1, data->data, data->len, 0, NULL);
        #else
        ret = (int)hal_i2c_simple_recv(s_i2c_config[device_id].id, data->dev_address, &data->reg_addr,
            1, data->data, data->len);
        #endif
        if (ret != 0) {
            LOGE("read i2c-%d timeout:%x ret:%d", device_id, data->dev_address, ret);
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
        .speed = I2C_MASTER_SPEED,
        .addr_as_slave = 0,
    };
    s_i2c_init_count[device_id]++;
    if (s_i2c_is_ready[device_id]) {
        LOG_PBUS("i2c reinit:%d", s_i2c_init_count[device_id]);
        return BSP_OK;
    }

    if (s_i2c_mux[device_id] == NULL) {
        MUTEX_INIT(&s_i2c_mux[device_id], dev_i2c_mutex);
    }
    MUTEX_LOCK(s_i2c_mux[device_id]);
    s_i2c_config[device_id].iomux_set();
    ret = (int)hal_i2c_open(s_i2c_config[device_id].id, &i2c_cfg);
    if (ret != BSP_OK) {
        LOGE("i2c_driver_install fail: %d", ret);
        goto ERROR_EXIT;
    }
    s_i2c_is_ready[device_id] = true;
    LOG_PBUS("i2c init ok");

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
