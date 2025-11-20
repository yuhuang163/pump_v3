/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "wf100d.h" // IWYU pragma: keep
#include "bsp_common.h"
#include "bsp_internal.h" // IWYU pragma: keep
#include "bsp_i2c.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define W100DP_I2C_ADDR                0x6D

#define W100DP_20KPA                   0
#define W100DP_40KPA                   1
#define W100DP_100KPA                  2
#define W100DP_200KPA                  3

#define WF100D_MEASUREMENT_RANG        2
#define PRESSURE_RAW_DATA_CALC(x)      (x) > 8388608 ? (((x) - 16777216) / 8388608.0f) : ((x) / 8388608.0f)

#if WF100D_MEASUREMENT_RANG ==  W100DP_100KPA
#define PRESSURE_CALC_KPA(x)           (x) * 50 + 5
#else
#define PRESSURE_CALC_KPA(x)           (x) * 125 - 12.5
#endif

#define TEMP_CALC_CELSIUS(msb, lsb)    -1.2 + (msb) + (lsb) * 0.004

#define W100DP_I2C_ID1                 I2C_MASTER_ID_1
#define W100DP_I2C_ID2                 I2C_MASTER_ID_2

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static air_pressue_status_t s_air_pressure[AIR_PRESSURE_NUM] = {0};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static int wf100d_write_nbyte(uint8_t index, i2c_master_data_t *data)
{
    int ret = 0;

    data->dev_address = W100DP_I2C_ADDR;
    for (uint8_t i = 0; i < 3; i++) {
        ret = bsp_i2c_write_byte(index == 0 ? W100DP_I2C_ID1 : W100DP_I2C_ID2, data);
        if (ret == BSP_OK) {
            break;
        }
        msleep(10);
    }

    return ret;
}

static int wf100d_read_nbyte(uint8_t index, i2c_master_data_t *data)
{
    int ret = 0;

    data->dev_address = W100DP_I2C_ADDR;
    for (uint8_t i = 0; i < 3; i++) {
        ret = bsp_i2c_read_byte(index == 0 ? W100DP_I2C_ID1 : W100DP_I2C_ID2, data);
        if (ret == BSP_OK) {
            break;
        }
        msleep(10);
    }

    return ret;
}

static int wf100d_read_nbyte_no_retry(uint8_t index, i2c_master_data_t *data)
{
    data->dev_address = W100DP_I2C_ADDR;
    return bsp_i2c_read_byte(index == 0 ? W100DP_I2C_ID1 : W100DP_I2C_ID2, data);
}

static int air_pressure_start_conversion(uint8_t index)
{
    uint8_t reg_val = WF100D_SINGLE_CONVERSION;
    i2c_master_data_t air_pressure = {
        .data = &reg_val,
        .len  = 1,
        .reg_addr = WF100D_CONVERSION_MODE_REG
    };

    return wf100d_write_nbyte(index, &air_pressure); 
}

static int wait_air_pressure_conversion_completed(uint8_t index)
{
    uint8_t reg_val = 0,  wait_timeout = 10;
    i2c_master_data_t air_pressure = {
        .data = &reg_val,
        .len  = 1,
        .reg_addr = WF100D_STATUS_REG
    };

    while(wait_timeout --) {
        wf100d_read_nbyte_no_retry(index, &air_pressure);
        if (reg_val == 1) {
            return BSP_OK;
        }
        msleep(1);
    }
    return BSP_TIMEOUT;
}

static int air_pressure_result_get(uint8_t index, air_pressue_status_t *out_status)
{
    uint8_t reg_data[5];
    int32_t raw_data = 0;
    i2c_master_data_t air_pressure = {
        .data = reg_data,
        .len  = 5,
        .reg_addr = WF100D_PRESSURE_MSB_REG
    };

    int ret = wf100d_read_nbyte(index, &air_pressure);
    if (ret != BSP_OK) {
        return ret;
    }
    raw_data = (int32_t)((((uint32_t)reg_data[0]) << 16) | (((uint32_t)reg_data[1]) << 8) | ((uint32_t)reg_data[2]));
    out_status->pressure = PRESSURE_CALC_KPA(PRESSURE_RAW_DATA_CALC(raw_data));
    out_status->temp = TEMP_CALC_CELSIUS(reg_data[3], reg_data[4]);
    out_status->raw_data = raw_data;
    return ret;
}

static int air_pressure_is_ready(uint8_t index)
{
    uint8_t reg_val = 0;
    int ret = BSP_NOT_READY;
    i2c_master_data_t air_pressure = {
        .data = &reg_val,
        .len  = 1,
        .reg_addr = WF100D_STATUS_REG
    };

    for (uint8_t i = 0; i < 3; i++) {
        ret = wf100d_read_nbyte(index, &air_pressure);
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
    bsp_i2c_init(W100DP_I2C_ID1);
    bsp_i2c_init(W100DP_I2C_ID2);
    msleep(10);
    for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (air_pressure_is_ready(i) != BSP_OK) {
            LOGE("air pressure is not ready: %d", i);
            return BSP_NOT_READY;
        }
    }
    return BSP_OK;
}

int bsp_air_pressure_deinit(void)
{
    return BSP_OK;
}

int bsp_read_air_pressure_status(void *args)
{
    air_pressue_status_t *out_status = (air_pressue_status_t *)args;
    // uint32_t start_time = get_timestamp();
    memset(out_status, 0, sizeof(air_pressue_status_t) * AIR_PRESSURE_NUM);
    for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (air_pressure_start_conversion(i) != BSP_OK) {
            continue;
        }
    }
    msleep(4);
    for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
        if (wait_air_pressure_conversion_completed(i) != BSP_OK) {
            continue;
        }
        air_pressure_result_get(i, &out_status[i]);
    }
    // PRINT_LOG("time: %d, pressure: %f, %f, raw: %d, %d\r\n", 
    //     get_timestamp_ms(start_time), out_status[0].pressure, out_status[1].pressure, out_status[0].raw_data, out_status[1].raw_data);
    return BSP_OK;
}
