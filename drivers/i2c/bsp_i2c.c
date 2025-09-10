/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "bsp_common.h"

/*
 * DEFINES
 *****************************************************************************************
 */
#define I2C_MASTER_TIMEOUT_MS(x)                    ((x > 20) ? ((x / 20) * 50) : 50)

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static bool s_i2c_is_ready = false;
static uint8_t s_i2c_init_count = 0;
static os_mutex_id s_i2c_mux = NULL;

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
int bsp_i2c_write_byte(uint8_t dev_address, uint8_t reg_addr, uint8_t *data, size_t len)
{
    int ret = 0;

    EXCEPTION_CHECK_INT(data);
    MUTEX_LOCK(s_i2c_mux);
    if (s_i2c_is_ready) {
        // TODO: write byte
    } else {
        LOGE("i2c is no ready"); 
    }
    MUTEX_UNLOCK(s_i2c_mux);
    if (ret != 0) {
        LOGE("read timeout:%x, ret:%d", dev_address, ret);
    }
    return ret;
}

int bsp_i2c_read_byte(uint8_t dev_address, uint8_t reg_addr, uint8_t *data, size_t len)
{
    int ret = 0;

    EXCEPTION_CHECK_INT(data);

    MUTEX_LOCK(s_i2c_mux);
    if (s_i2c_is_ready) {
        // TODO: read byte
    } else {
        LOGE("i2c is no ready");  
    }
    MUTEX_UNLOCK(s_i2c_mux);
    if (ret != 0) {
        LOGE("write timeout:%x ret:%d", dev_address, ret);
    }

    return ret;
}

int bsp_i2c_init(void)
{
    s_i2c_init_count++;
    if (s_i2c_is_ready) {
        LOG_PBUS("i2c reinit:%d", s_i2c_init_count);
        return BSP_OK;
    }

    if (s_i2c_mux == NULL) {
        MUTEX_INIT(&s_i2c_mux, dev_i2c_mutex);
    }
    
    MUTEX_LOCK(s_i2c_mux);
    int ret = 0;
    // TODO: init i2c
    if (ret != BSP_OK) {
        LOGE("i2c_driver_install fail: %d", ret);
        goto ERROR_EXIT;
    }
    s_i2c_is_ready = true;
    LOG_PBUS("i2c init ok");

ERROR_EXIT:
    MUTEX_UNLOCK(s_i2c_mux);
    
    return BSP_OK;
}

int bsp_i2c_deinit(void)
{
    MUTEX_LOCK(s_i2c_mux);
    s_i2c_is_ready = 0;
    int ret = 0;
    // TODO: deinit i2c
    MUTEX_UNLOCK(s_i2c_mux);
    LOGE("i2c_driver_delete:%d", ret);

    return ret;

}
