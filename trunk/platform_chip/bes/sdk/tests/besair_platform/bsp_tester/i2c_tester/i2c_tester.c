/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "i2c_tester.h"

#define I2C_CMD(n)              {TO_STRING(n), CONCAT(i2c_tester_, n)},
#define I2C_CMD_TESTER(n)                       \
static void CONCAT(i2c_tester_, n)()            \
{                                               \
    switch (mode) {                             \
        case HAL_I2C_API_MODE_TASK:             \
            CONCAT(i2c_tester_task_, n)();      \
            break;                              \
        case HAL_I2C_API_MODE_SIMPLE:           \
            CONCAT(i2c_tester_simple_, n)();    \
            break;                              \
        default:                                \
            PARAM_CHK(false);                   \
    }                                           \
}
#define I2C_DEFAULT_ADDR        0x27
#define I2C_BUF_SIZE            20
#define SWAP_16(n)              ((((n) & 0x00ffUL) << 8) |      \
                                 (((n) & 0xff00UL) >> 8))
#define SWAP_32(n)              ((((n) & 0x000000ffUL) << 24) | \
                                 (((n) & 0x0000ff00UL) <<  8) | \
                                 (((n) & 0x00ff0000UL) >>  8) | \
                                 (((n) & 0xff000000UL) >> 24))

static enum HAL_I2C_ID_T id;
static enum HAL_I2C_API_MODE_T mode;
static enum HAL_I2C_ACTION_AFTER_WRITE_T after = HAL_I2C_RESTART_AFTER_WRITE;
static uint32_t rate;
static uint32_t addr = I2C_DEFAULT_ADDR;
static uint8_t  buf[I2C_BUF_SIZE];
static size_t   reg, data, r, d;
static uint32_t reg_len, data_len;
static uint32_t ret;

static void i2c_tester_task_write()
{
    switch (reg_len) {
        case 1:  break;
        case 2:  r = SWAP_16(reg); break;
        case 4:  r = SWAP_32(reg); break;
        default: PARAM_CHK(false);
    }
    switch (data_len) {
        case 1:  break;
        case 2:  d = SWAP_16(data); break;
        case 4:  d = SWAP_32(data); break;
        default: PARAM_CHK(false);
    }
    memset(buf, 0, sizeof(buf));
    memcpy(buf, (uint8_t *)&r, reg_len);
    memcpy((uint8_t *)(buf + reg_len), (uint8_t *)&d, data_len);

    CMD_TRACE("i2c%d [0x%08X][W]:\t0x%0X", id, reg, data);
    ret = hal_i2c_send(id, addr, buf, reg_len, data_len, 0, NULL);
    ASSERT(!ret, "i2c%d send error", id);
}

static void i2c_tester_simple_write()
{
    const uint8_t buffer[3] = {0xA1, 0xA3, 0xA5};

    CMD_TRACE("i2c%d is sending messages", id);
    ret = hal_i2c_simple_send(id, (uint16_t)addr, buffer, ARRAY_SIZE(buf));
    ASSERT(ret, "i2c%d send error", id);
}

static void i2c_tester_task_read()
{
    switch (reg_len) {
        case 1:
        case 2:  break;
        case 4:  r = SWAP_32(reg); break;
        default: PARAM_CHK(false);
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, (uint8_t *)&r, reg_len);

    hal_i2c_recv(id, addr, buf, reg_len, data_len, after, 0, 0);

    memcpy((uint8_t *)&d, (uint8_t *)(buf + reg_len), data_len);
    switch (data_len) {
        case 1:  break;
        case 2:  data = SWAP_16(d); break;
        case 4:  data = SWAP_32(d); break;
        default: PARAM_CHK(false);
    }
    CMD_TRACE("i2c%d [0x%04X][R]:\t0x%04X", id, reg, data);
}

static void i2c_tester_simple_read()
{
    const uint8_t buffer[3] = {0,};
    volatile uint16_t dev_id = 0;

    hal_i2c_simple_recv(id, addr, buffer, 1, (uint8_t *)&dev_id, 2);
    dev_id = SWAP_16(dev_id);
    CMD_TRACE("i2c%d device id[R]:\t\t0x%04x", id, dev_id);
}

I2C_CMD_TESTER(write)
I2C_CMD_TESTER(read)

static const i2c_tester_handler_t i2c_tester_handler[] = {
    I2C_CMD(write)
    I2C_CMD(read)
};

static void i2c_tester_i2c_init(void)
{
    /* i2c port related attributions setting */
    struct HAL_I2C_CONFIG_T i2cAttr = {
        .mode               = mode,
        .use_sync           = true,
        .use_dma            = false,
        .as_master          = true,
        .rising_time_ns     = 0,
        .speed              = rate * 1000,
        .addr_as_slave      = 0,
    };

    /* i2c port initialization */
    switch (id) {
        case HAL_I2C_ID_0:
            hal_iomux_set_i2c0(); break;
#ifdef I2C1_BASE
        case HAL_I2C_ID_1:
            hal_iomux_set_i2c1(); break;
#endif
#ifdef I2C2_BASE
        case HAL_I2C_ID_2:
            hal_iomux_set_i2c2(); break;
#endif
#ifdef I2C3_BASE
        case HAL_I2C_ID_3:
            hal_iomux_set_i2c3(); break;
#endif
        default:
            PARAM_CHK(false);
    }
    hal_i2c_close(id);
    ret = hal_i2c_open(id, &i2cAttr);
    ASSERT(!ret, "i2c port configuration error");
}

void i2c_tester(const char **params)
{
    ASSIGN_VAL(id,       params[0]);
    ASSIGN_VAL(rate,     params[1]);
    ASSIGN_VAL(mode,     params[2]);
    STR_TO_HEX(addr,     params[4]);
    STR_TO_HEX(reg,      params[5]);
    STR_TO_HEX(data,     params[7]);
    ASSIGN_VAL(reg_len,  params[6]);
    ASSIGN_VAL(data_len, params[8]);

    i2c_tester_i2c_init();

    CMD_HANDLER(i2c_tester, params[3]);
}