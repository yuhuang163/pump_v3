/***************************************************************************
 *
 * Copyright 2015-2021 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifdef I2C_DEBUG_SLAVE_TEST

#include <string.h>
#include <stdlib.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_i2c.h"
#include "hal_sysfreq.h"

#if 1
    #define I2C_DEBUG_SLAVE_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define I2C_DEBUG_SLAVE_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
    #define I2C_DEBUG_SLAVE_DUMP16(n, s, ...)     DUMP16(n, s, ##__VA_ARGS__)
#else
    #define I2C_DEBUG_SLAVE_TRACE(n, s, ...)      TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define I2C_DEBUG_SLAVE_DUMP8(n, s, ...)
    #define I2C_DEBUG_SLAVE_DUMP16(n, s, ...)
#endif

#if 1
    #define I2C_DEBUG_SLAVE_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define I2C_DEBUG_SLAVE_DELAY_US(us)          hal_sys_timer_delay_us(us)
#else
    #define I2C_DEBUG_SLAVE_DELAY_MS(ms)
    #define I2C_DEBUG_SLAVE_DELAY_US(us)
#endif

//chip id list
#define CHIP_ID_2001                    0x2011
#define CHIP_ID_2002                    0x2021
#define CHIP_ID_1501                    0x1510
#define CHIP_ID_1502X                   0x1520
#define CHIP_ID_2003                    0x2031
#define CHIP_ID_1600                    0x1600

//chip id i2c address list
#define CHIP_ID_I2C_ADDR_2001           0x17
#define CHIP_ID_I2C_ADDR_2002           0x17
#define CHIP_ID_I2C_ADDR_1501           0x27
#define CHIP_ID_I2C_ADDR_1502X          0x27
#define CHIP_ID_I2C_ADDR_2003           0x17
#define CHIP_ID_I2C_ADDR_1600           0x27

//Select the chip to be read and written
#define CHIP_ID                         CHIP_ID_1502X

#define I2C_MASTER_NUMBER               3   /*0:i2c0
                                              1:i2c1
                                              2:i2c2
                                              3:i2c3
                                              4:i2c4
                                              5:i2c5
                                              6:i2c6
                                              7:i2c7
                                              8:i2c8
                                              9:gpio i2c,Cannot be used because the slave protocol cannot be implemented
                                            */

/*
The task mode API does not support the slave write
protocol (controlling the restart and stop bits of i2c),
so it cannot be used, and can only be used in simple mode,tanwenchen,2021-09-29
*/
//#define I2C_MODE_TASK     //!!!Warnning: Cannot open when writing i2c slave

#define BUFFER_SIZE_MAX     256
static uint8_t i2c_buffer[BUFFER_SIZE_MAX + 1] = {0};

#if (I2C_MASTER_NUMBER < 9)
    static uint8_t i2c_id = HAL_I2C_ID_0;
#else
    static struct HAL_GPIO_I2C_CONFIG_T cfg;
#endif

static uint32_t i2c_read(uint8_t device_addr, uint8_t *reg_buf, uint16_t reg_len, uint8_t *bufp, uint16_t read_len)
{
    uint32_t ret = 20210929;

    if (reg_len > BUFFER_SIZE_MAX / 2) {
        reg_len = BUFFER_SIZE_MAX / 2;
    }
    if (read_len > BUFFER_SIZE_MAX / 2) {
        read_len = BUFFER_SIZE_MAX / 2;
    }

    memset(i2c_buffer, 0, sizeof(i2c_buffer));
    memcpy((uint8_t *)i2c_buffer, (uint8_t *)reg_buf, reg_len);

#if (I2C_MASTER_NUMBER == 9)
    ret = hal_gpio_i2c_recv(&cfg, device_addr, i2c_buffer, reg_len, read_len, 0);
#else
#ifdef I2C_MODE_TASK
    //Task mode API,2021-03-30
    ret = hal_i2c_recv((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, read_len, HAL_I2C_RESTART_AFTER_WRITE, 0, 0);
#else
    //simple mode API,2021-03-31
    ret = hal_i2c_simple_recv((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, i2c_buffer + reg_len, read_len);
#endif
#endif

    memcpy((uint8_t *)bufp, (uint8_t *)(i2c_buffer + reg_len), read_len);

    if (ret) {
        I2C_DEBUG_SLAVE_TRACE(3, "***%s:%d,return error:0x%X", __func__, __LINE__, ret);
    }
    return ret;
}

static uint32_t i2c_write(uint8_t device_addr, uint8_t *reg_buf, uint16_t reg_len, uint8_t *bufp, uint16_t write_len)
{
    uint32_t ret = 20210929;

    if (reg_len > BUFFER_SIZE_MAX / 2) {
        reg_len = BUFFER_SIZE_MAX / 2;
    }
    if (write_len > BUFFER_SIZE_MAX / 2) {
        write_len = BUFFER_SIZE_MAX / 2;
    }

    memset(i2c_buffer, 0, sizeof(i2c_buffer));
    memcpy((uint8_t *)i2c_buffer, (uint8_t *)reg_buf, reg_len);
    memcpy((uint8_t *)(i2c_buffer + reg_len), (uint8_t *)bufp, write_len);

#if (I2C_MASTER_NUMBER == 9)
    ret = hal_gpio_i2c_send(&cfg, device_addr, i2c_buffer, reg_len, write_len);
#else
#ifdef I2C_MODE_TASK
    //Task mode API,2021-03-30
    ret = hal_i2c_send((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, write_len, 0, NULL);
#else
    //simple mode API,2021-03-31
    if (reg_len > 1) {
        ret = hal_i2c_mst_write((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, NULL, true, false, false);
        ret = hal_i2c_mst_write((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer + reg_len, write_len, NULL, true, true, false);
    } else {
        ret = hal_i2c_simple_send((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len + write_len);
    }
#endif
#endif

    if (ret) {
        I2C_DEBUG_SLAVE_TRACE(3, "***%s:%d,return error:0x%X", __func__, __LINE__, ret);
    }
    return ret;
}

#define BigLittleSwap16(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define BigLittleSwap32(x) ((((x) & 0x000000ffUL) << 24) | \
                            (((x) & 0x0000ff00UL) <<  8) | \
                            (((x) & 0x00ff0000UL) >>  8) | \
                            (((x) & 0xff000000UL) >> 24))

POSSIBLY_UNUSED static uint32_t i2c_read_reg8_data16(uint8_t slave_addr, uint8_t reg_addr, uint16_t *data)
{
    uint8_t temp = reg_addr;
    uint32_t ret;

    ret = i2c_read(slave_addr, (uint8_t *)&temp, 1, (uint8_t *)data, 2);
    *data = BigLittleSwap16(*data);

    return ret;
}

POSSIBLY_UNUSED static uint32_t i2c_read_reg16_data16(uint8_t slave_addr, uint16_t reg_addr, uint16_t *data)
{
    uint16_t temp = reg_addr;
    uint32_t ret;

    ret = i2c_read(slave_addr, (uint8_t *)&temp, 2, (uint8_t *)data, 2);
    *data = BigLittleSwap16(*data);

    return ret;
}

POSSIBLY_UNUSED static uint32_t i2c_read_reg32_data32(uint8_t slave_addr, uint32_t reg_addr, uint32_t *data)
{
    uint32_t temp = reg_addr;
    uint32_t ret;

    temp = BigLittleSwap32(temp);
    ret = i2c_read(slave_addr, (uint8_t *)&temp, 4, (uint8_t *)data, 4);
    *data = BigLittleSwap32(*data);

    return ret;
}

POSSIBLY_UNUSED static uint32_t i2c_send_reg8_data16(uint8_t slave_addr, uint8_t reg_addr, uint16_t data)
{
    uint8_t temp = reg_addr;
    uint16_t temp_data = data;

    temp_data = BigLittleSwap16(temp_data);
    return i2c_write(slave_addr, (uint8_t *)&temp, 1, (uint8_t *)&temp_data, 2);
}

POSSIBLY_UNUSED static uint32_t i2c_send_reg32_data32(uint8_t slave_addr, uint32_t reg_addr, uint32_t data)
{
    uint32_t temp = reg_addr;
    uint32_t temp_data = data;

    temp = BigLittleSwap32(temp);
    temp_data = BigLittleSwap32(temp_data);
    return i2c_write(slave_addr, (uint8_t *)&temp, 4, (uint8_t *)&temp_data, 4);
}

void i2c_debug_slave_test(void)
{
    uint16_t ChipID = 0;
    uint32_t val = 0;

    I2C_DEBUG_SLAVE_TRACE(0, "  ");
    I2C_DEBUG_SLAVE_TRACE(0, "------i2c debug slave read/write test started------");
    I2C_DEBUG_SLAVE_TRACE(0, "  ");

    //i2c master config
#if (I2C_MASTER_NUMBER == 9)
    cfg.scl = HAL_GPIO_PIN_P0_2;
    cfg.sda = HAL_GPIO_PIN_P0_3;
    cfg.speed = 100 * 1000;
    hal_gpio_i2c_open(&cfg);
#else
    struct HAL_I2C_CONFIG_T cfg;
    memset(&cfg, 0, sizeof(cfg));
#ifdef I2C_MODE_TASK
    cfg.mode = HAL_I2C_API_MODE_TASK;
#else
    cfg.mode = HAL_I2C_API_MODE_SIMPLE;
#endif
    cfg.use_dma = 0;
    cfg.use_sync = 1;
    cfg.speed = 100 * 1000;
    cfg.as_master = 1;

    switch (I2C_MASTER_NUMBER) {
        case HAL_I2C_ID_0: {
            i2c_id = HAL_I2C_ID_0;
            hal_iomux_set_i2c0();
            break;
        }
#ifdef I2C1_BASE
        case HAL_I2C_ID_1: {
            i2c_id = HAL_I2C_ID_1;
            hal_iomux_set_i2c1();
            break;
        }
#endif
#ifdef I2C2_BASE
        case HAL_I2C_ID_2: {
            i2c_id = HAL_I2C_ID_2;
            hal_iomux_set_i2c2();
            break;
        }
#endif
#ifdef I2C3_BASE
        case HAL_I2C_ID_3: {
            i2c_id = HAL_I2C_ID_3;
            hal_iomux_set_i2c3();
            break;
        }
#endif
#ifdef I2C4_BASE
        case HAL_I2C_ID_4: {
            i2c_id = HAL_I2C_ID_4;
            hal_iomux_set_i2c4();
            break;
        }
#endif
#ifdef I2C5_BASE
        case HAL_I2C_ID_5: {
            i2c_id = HAL_I2C_ID_5;
            hal_iomux_set_i2c5();
            break;
        }
#endif
#ifdef I2C6_BASE
        case HAL_I2C_ID_6: {
            i2c_id = HAL_I2C_ID_6;
            hal_iomux_set_i2c6();
            break;
        }
#endif
#ifdef I2C7_BASE
        case HAL_I2C_ID_7: {
            i2c_id = HAL_I2C_ID_7;
            hal_iomux_set_i2c7();
            break;
        }
#endif
#ifdef I2C8_BASE
        case HAL_I2C_ID_8: {
            i2c_id = HAL_I2C_ID_8;
            hal_iomux_set_i2c8();
            break;
        }
#endif
        default: {
            ASSERT(false, "%s:%d,error i2c id:%d", __func__, __LINE__, I2C_MASTER_NUMBER);
            break;
        }
    }

    hal_i2c_open((enum HAL_I2C_ID_T)i2c_id, &cfg);
#endif

    while (1) {
#if     (CHIP_ID == CHIP_ID_2001)
        i2c_read_reg8_data16(CHIP_ID_I2C_ADDR_2001, 0, &ChipID);
#elif   (CHIP_ID == CHIP_ID_2002)
        i2c_read_reg16_data16(CHIP_ID_I2C_ADDR_2002, 0, &ChipID);
#elif   (CHIP_ID == CHIP_ID_1501)
        i2c_read_reg8_data16(CHIP_ID_I2C_ADDR_1501, 0, &ChipID);
#elif   (CHIP_ID == CHIP_ID_1502X)
        i2c_read_reg8_data16(CHIP_ID_I2C_ADDR_1502X, 0, &ChipID);
#elif   (CHIP_ID == CHIP_ID_1600)
        i2c_read_reg8_data16(CHIP_ID_I2C_ADDR_1600, 0, &ChipID);
#elif   (CHIP_ID == CHIP_ID_2003)
        i2c_read_reg16_data16(CHIP_ID_I2C_ADDR_2003, 0, &ChipID);
#endif
        I2C_DEBUG_SLAVE_TRACE(0, "  ");
        I2C_DEBUG_SLAVE_TRACE(0, "---chip id:0x%X", ChipID);

        i2c_read_reg32_data32(0x11, 0x30000000, &val);
        I2C_DEBUG_SLAVE_TRACE(1, "read  ram[0x30000000]=0x%08X", val);

        i2c_read_reg32_data32(0x11, 0x40086004, &val);
        I2C_DEBUG_SLAVE_TRACE(1, "read  reg[0x40086004]=0x%08X", val);

        val = 0xAA552277;
        i2c_send_reg32_data32(0x11, 0x40086004, val);
        I2C_DEBUG_SLAVE_TRACE(1, "write reg[0x40086004]=0x%08X", val);

        i2c_read_reg32_data32(0x11, 0x40086004, &val);
        I2C_DEBUG_SLAVE_TRACE(1, "read  reg[0x40086004]=0x%08X", val);

        I2C_DEBUG_SLAVE_DELAY_MS(1000);
    }
}

#endif

