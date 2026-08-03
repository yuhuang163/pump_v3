/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "limscg04pcns5.h" // IWYU pragma: keep
#include "bsp_common.h"
#include "bsp_config.h"
#include "bsp_internal.h" // IWYU pragma: keep
#include "bsp_i2c.h" // IWYU pragma: keep
#include "hal_air_pressure.h"

/*
* DEFINES
*****************************************************************************************
*/
#define LIMS_I2C_ADDR                  0x6C
#define LIMS_LOG_DEBUG                 0
#define LIMS_ABNORMAL_MAX_COUNT        500
#define LIMS_LOG_COUNT_INTERVAL        100

#define PRESSURE_RAW_DATA_CALC(x)  (x) > 8388608 ? \
 (((x) - 16777216) / 8388608.0f * (-110)) : ((x) / 8388608.0f * (-110))

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static float s_temp[AIR_PRESSURE_NUM] = {0};
static uint8_t s_temp_is_ready[AIR_PRESSURE_NUM] = {0};
static uint8_t s_dev_is_ready[AIR_PRESSURE_NUM] = {0};
static uint16_t s_dev_abnormal_count[AIR_PRESSURE_NUM] = {0};
static uint16_t s_log_count = 0;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static int lims_write_nbyte(uint8_t index, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;

    if (index >= AIR_PRESSURE_NUM) {
        LOGE("air pressure index is invalid: %d", index);
        return BSP_INVALID_PARAMETER;
    }

    if (s_dev_is_ready[index] == 0) {
        if (s_log_count % LIMS_LOG_COUNT_INTERVAL == 0) {
            LOGE("air pressure is not ready: %d", index);
        }
        return BSP_NOT_READY;
    }
    
    i2c_master_data_t i2c_data = {
        .dev_address = LIMS_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    for (uint8_t i = 0; i < 3; i++) {
        ret = bsp_i2c_write_byte(index == 0 ? AIR_PRESSURE_I2C_ID1 : AIR_PRESSURE_I2C_ID2, &i2c_data);
        if (ret == BSP_OK) {
            s_dev_abnormal_count[index] = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_dev_abnormal_count[index]++;
    if (s_dev_abnormal_count[index] > LIMS_ABNORMAL_MAX_COUNT) {
        s_dev_is_ready[index] = 0;
    } else {
        LOG_PBUS("air pressure %d abnormal count: %d", index, s_dev_abnormal_count[index]);
    }

    return BSP_ERROR;
}

static int lims_read_nbyte(uint8_t index, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;
    
    if (index >= AIR_PRESSURE_NUM) {
        LOGE("air pressure index is invalid: %d", index);
        return BSP_INVALID_PARAMETER;
    }
    
    if (s_dev_is_ready[index] == 0) {
        if (s_log_count % LIMS_LOG_COUNT_INTERVAL == 0) {
            LOGE("air pressure is not ready: %d", index);
        }
        return BSP_NOT_READY;
    }

    i2c_master_data_t i2c_data = {
        .dev_address = LIMS_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    for (uint8_t i = 0; i < 3; i++) {
        ret = bsp_i2c_read_byte(index == 0 ? AIR_PRESSURE_I2C_ID1 : AIR_PRESSURE_I2C_ID2, &i2c_data);
        if (ret == BSP_OK) {
            s_dev_abnormal_count[index] = 0;
            return BSP_OK;
        }
        msleep(10);
    }

    s_dev_abnormal_count[index]++;
    if (s_dev_abnormal_count[index] > LIMS_ABNORMAL_MAX_COUNT) {
        s_dev_is_ready[index] = 0;
    } else {
        LOG_PBUS("air pressure %d abnormal count: %d", index, s_dev_abnormal_count[index]);
    }

    return BSP_ERROR;
}

static int lims_read_nbyte_no_retry(uint8_t index, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int ret = 0;
    if (index >= AIR_PRESSURE_NUM) {
        LOGE("air pressure index is invalid: %d", index);
        return BSP_INVALID_PARAMETER;
    }

    i2c_master_data_t i2c_data = {
        .dev_address = LIMS_I2C_ADDR,
        .reg_addr = &reg_addr,
        .reg_addr_len = 1,
        .data = data,
        .len = len,
    };

    ret = bsp_i2c_read_byte(index == 0 ? AIR_PRESSURE_I2C_ID1 : AIR_PRESSURE_I2C_ID2, &i2c_data);
    if (ret == BSP_OK) {
        s_dev_abnormal_count[index] = 0;
        return BSP_OK;
    }

    s_dev_abnormal_count[index]++;
    if (s_dev_abnormal_count[index] > LIMS_ABNORMAL_MAX_COUNT) {
        s_dev_is_ready[index] = 0;
    } else {
        LOG_PBUS("air pressure %d abnormal count: %d", index, s_dev_abnormal_count[index]);
    }

    return BSP_ERROR;
}

static int start_air_pressure_conversion(lims_conversion_data_type_e type)
{
    uint8_t reg_val = (type == LIMS_PRESSURE_DATA) ? 
            LIMS_1024X_PRESSURE_CONVERSION : LIMS_1024X_TEMP_CONVERSION;
    for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (lims_write_nbyte(i, LIMS_CONVERSION_MODE_REG, &reg_val, 1) != BSP_OK) {
            return BSP_ERROR;
        }
    }
    return BSP_OK;
}

static int wait_conversion_completed(void)
{
    uint8_t reg_val = 0,  wait_timeout = 0, index = 0;
    for (;;) {
        if (lims_read_nbyte_no_retry(index, LIMS_STATUS_REG, &reg_val, 1) != BSP_OK) {
            return BSP_ERROR; /// 读取失败
        }
        /// 转换完成
        if (reg_val == 1) {
            index++;
            if (index >= AIR_PRESSURE_NUM) {
                break; /// 所有设备都已读取完成
            }
            continue; /// 继续读取下一个设备
        }
        wait_timeout++;
        if (wait_timeout > 10) {
            return BSP_TIMEOUT; /// 等待超时
        }
        msleep(1);
    }
    if (wait_timeout > 0) {
        LOG_PBUS("wait conversion completed timeout: %d", wait_timeout);
    }
    return BSP_OK;
}

static int air_pressure_result_get(uint8_t index, air_pressue_status_t *out_status)
{
    uint8_t reg_data[4];
    int32_t raw_data = 0;
    int ret = lims_read_nbyte(index, LIMS_PRESSURE_MSB_REG, reg_data, 3);
    if (ret != BSP_OK) {
        return ret;
    }
    raw_data = (int32_t)((((uint32_t)reg_data[0]) << 16) | (((uint32_t)reg_data[1]) << 8) | ((uint32_t)reg_data[2]));
    out_status->pressure = PRESSURE_RAW_DATA_CALC(raw_data);
    out_status->temp = 0;
    out_status->raw_data = raw_data;
    return ret;
}

static float temperature_result_get(uint8_t index)
{
    uint8_t reg_data[4];

    if (lims_read_nbyte(index, LIMS_TEMP_MSB_REG, reg_data, 2) != BSP_OK) {
        return 0;
    }
    
    int16_t temp_raw_data = (int16_t)((reg_data[0] << 8) | reg_data[1]);
    float temp = temp_raw_data > 0 ? (temp_raw_data / 128.0f) : ((temp_raw_data & 0x7FFF) / 128.0f - 256.0f);
#if LIMS_LOG_DEBUG
    BSP_LOG_PASS("temp: %f, raw: %d\r\n", temp, temp_raw_data);
#endif
    return temp;
}

static int temperature_single_conversion(void)
{
    if (start_air_pressure_conversion(LIMS_TEMP_DATA) != BSP_OK) {
        LOGE("conversion failed");
        return BSP_ERROR;
    }
    msleep(LIMS_TEMP_CONVERSION_TIME);
    if (wait_conversion_completed() != BSP_OK) {
        LOGE("conversion timeout");
        return BSP_ERROR;
    }

    for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
        s_temp[i] = temperature_result_get(i);
        s_temp_is_ready[i] = true;
    } 
    LOG_PBUS("temperature conversion success");
    return BSP_OK;
}

static int air_pressure_is_ready(uint8_t index)
{
    uint8_t reg_val = 0;
    int ret = BSP_NOT_READY;
    for (uint8_t i = 0; i < 3; i++) {
        ret = lims_read_nbyte_no_retry(index, LIMS_STATUS_REG, &reg_val, 1);
        if (ret == BSP_OK) {
            break;
        }
        msleep(5);
    }
    return ret != BSP_OK ? BSP_NOT_READY : BSP_OK;
}

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_air_pressure_init(void)
{
    for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (air_pressure_is_ready(i) != BSP_OK) {
            LOGE("air pressure is not ready: %d", i);
            return BSP_NOT_READY;
        } else {
            s_dev_is_ready[i] = true;
            s_dev_abnormal_count[i] = 0;
        }
    }

    if (temperature_single_conversion() != BSP_OK) {
        return BSP_ERROR;
    }
    start_air_pressure_conversion(LIMS_PRESSURE_DATA);
    LOG_PBUS("air pressure init success");
    return BSP_OK;
}

int bsp_air_pressure_deinit(void)
{
    memset(s_temp, 0, sizeof(s_temp));
    memset(s_temp_is_ready, 0, sizeof(s_temp_is_ready));
    memset(s_dev_is_ready, 0, sizeof(s_dev_is_ready));
    memset(s_dev_abnormal_count, 0, sizeof(s_dev_abnormal_count));
    s_log_count = 0;
    return BSP_OK;
}

int bsp_read_air_pressure_status(void *args)
{
    air_pressue_status_t *out_status = (air_pressue_status_t *)args;
#if LIMS_LOG_DEBUG
    uint32_t start_time = get_timestamp();
#endif
    if (s_dev_is_ready[0] == 0 || s_dev_is_ready[1] == 0) {
        return BSP_NOT_READY;
    }
    
    memset(out_status, 0, sizeof(air_pressue_status_t) * AIR_PRESSURE_NUM);
    /// 温度转换未完成，则进行单次转换
    if (s_temp_is_ready[0] == 0 || s_temp_is_ready[1] == 0) {
        if (temperature_single_conversion() != BSP_OK) {
            return BSP_ERROR;
        }
        LOG_PBUS("start pressure conversion");
        start_air_pressure_conversion(LIMS_PRESSURE_DATA); /// 重新开始压力转换
        msleep(LIMS_PRESSURE_CONVERSION_TIME); /// 等待压力转换完成
    }

    /// 等待转换完成
    if (wait_conversion_completed() != BSP_OK) {
        start_air_pressure_conversion(LIMS_PRESSURE_DATA); /// 重新开始压力转换
        return BSP_ERROR;
    }

    for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (air_pressure_result_get(i, &out_status[i]) != BSP_OK) {
            start_air_pressure_conversion(LIMS_PRESSURE_DATA); /// 重新开始压力转换
            return BSP_ERROR;
        }
    }
    start_air_pressure_conversion(LIMS_PRESSURE_DATA);
#if LIMS_LOG_DEBUG
    BSP_LOG_PASS("4 time: %d, pressure: %f, %f, raw: %d, %d\r\n", 
        get_timestamp_ms(start_time), out_status[0].pressure, 
        out_status[1].pressure, out_status[0].raw_data, out_status[1].raw_data);
#endif
    s_log_count++;
    return BSP_OK;
}

int bsp_air_pressure_is_ready(void *args)
{
    uint8_t *pressure_is_ready_status = (uint8_t *)args;
    for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
        pressure_is_ready_status[i] = s_dev_is_ready[i];
    }
    return BSP_OK;
}   
