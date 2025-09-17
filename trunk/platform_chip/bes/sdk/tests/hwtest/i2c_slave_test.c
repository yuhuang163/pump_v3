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
#ifdef I2C_SLAVE_TEST

#include <string.h>
#include <stdlib.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_i2c.h"
#include "hal_sysfreq.h"

#if 1
    #define I2C_SLAVE_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define I2C_SLAVE_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
#else
    #define I2C_SLAVE_TRACE(n, s, ...)      TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define I2C_SLAVE_DUMP8(n, s, ...)
#endif

#if 1
    #define I2C_SLAVE_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
#else
    #define I2C_SLAVE_DELAY_MS(ms)
#endif

/****************************************************************************
Key macro: COMPILE_MASTER_CODE
Open     : The master code is compiled (default)
Closed   : The slave code is compiled
****************************************************************************/
//#define COMPILE_MASTER_CODE
//#define I2C_MODE_TASK
//#define I2C_MODE_DMA
//#define I2C_USE_SYNC
//#define I2C_LOOP_MODE

#define I2C_MASTER_NUMBER               0   /*0:i2c0
                                              1:i2c1
                                              2:i2c2
                                              3:i2c3
                                              4:i2c4
                                              5:i2c5
                                              6:i2c6
                                              7:i2c7
                                              8:i2c8
                                              9:gpio i2c
                                            */
#define I2C_SLAVE_NUMBER                0   /*0:i2c0
                                              1:i2c1
                                              2:i2c2
                                              3:i2c3
                                              4:i2c4
                                              5:i2c5
                                              6:i2c6
                                              7:i2c7
                                              8:i2c8
                                              9:gpio i2c
                                            */
#define I2C_SLAVE_ADDR                  0x2A

#define REG_LEN 1
#define BUF_LEN 16

#ifdef COMPILE_MASTER_CODE
    static uint8_t master_reg_buf[REG_LEN] = {0x55};
    static uint8_t master_data_buf[BUF_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
#ifdef I2C_LOOP_MODE
    static uint8_t slave_data_buf[BUF_LEN] = {0};
#endif
#else
    static uint32_t slave_rx_cnt = REG_LEN + BUF_LEN;
    static uint8_t slave_data_buf[REG_LEN + BUF_LEN] = {0};
#endif

#if ((I2C_MASTER_NUMBER < 9) || (I2C_SLAVE_NUMBER < 9))
    static uint8_t i2c_id = HAL_I2C_ID_0;
#else
    static struct HAL_GPIO_I2C_CONFIG_T cfg;
#endif

#if defined(I2C_MODE_TASK)
static volatile uint8_t i2c_txrx_complete = 0;
static uint32_t err_code = 0;

#ifdef COMPILE_MASTER_CODE
POSSIBLY_UNUSED static void i2c_txrx_complete_handler(enum HAL_I2C_ID_T id, uint32_t transfer_id,
                                      const uint8_t *tx_buf, uint32_t tx_len,
                                      const uint8_t *rx_buf, uint32_t rx_len,
                                      enum HAL_I2C_ERRCODE_T errcode)
{
    I2C_SLAVE_TRACE(7, "i2c%d tx/rx done, transfer id:%d, tx buf:0x%X, tx len:%d, rx buf:0x%X, rx len:%d, errcode:0x%X", id, transfer_id,
                    (uint32_t)tx_buf, tx_len, (uint32_t)rx_buf, rx_len, errcode);
    i2c_txrx_complete = 1;
    err_code = errcode;
}

#else

static uint32_t i2c_rx_len = 0;
static uint32_t i2c_transport_dir = 0;
POSSIBLY_UNUSED static void i2c_slave_txrx_complete_handler(enum HAL_I2C_ID_T id, uint32_t transport_dir,
                                                            const uint8_t *tx_buf, uint32_t tx_len,
                                                            const uint8_t *rx_buf, uint32_t rx_len,
                                                            enum HAL_I2C_ERRCODE_T errcode)
{
    if (transport_dir == 0) {
        I2C_SLAVE_TRACE(7, "transport_dir:%d, i2c%d: send done", transport_dir, id);
    } else {
        I2C_SLAVE_TRACE(7, "transport_dir:%d, i2c%d: recv done", transport_dir, id);
    }
    I2C_SLAVE_TRACE(7, "tx buf:0x%X, tx len:%d, rx buf:0x%X, rx len:%d, errcode:0x%X",
                    (uint32_t)tx_buf, tx_len, (uint32_t)rx_buf, rx_len, errcode);
    i2c_transport_dir = transport_dir;
    i2c_txrx_complete = 1;
    i2c_rx_len = rx_len;
    err_code = errcode;
}

POSSIBLY_UNUSED static void i2c_read_req_handler(enum HAL_I2C_ID_T id, uint32_t reg_addr, uint8_t **i2c_tx_buf, uint32_t *i2c_tx_len, enum HAL_I2C_ERRCODE_T errcode)
{
#ifdef I2C_LOOP_MODE
    *i2c_tx_buf = &slave_data_buf[1];
    *i2c_tx_len = sizeof(slave_data_buf) - 1;
#else
    *i2c_tx_buf = slave_data_buf;
    *i2c_tx_len = sizeof(slave_data_buf);
#endif
    I2C_SLAVE_TRACE(0, "%s:%d: reg_addr 0x%02X", __func__, id, reg_addr);
}
#endif
#endif

#ifdef COMPILE_MASTER_CODE
#define BUFFER_SIZE_MAX     256
static uint8_t i2c_buffer[BUFFER_SIZE_MAX + 1] = {0};

POSSIBLY_UNUSED static uint32_t i2c_read(uint8_t device_addr, uint8_t *reg_buf, uint16_t reg_len, uint8_t *bufp, uint16_t read_len)
{
    uint32_t ret = 20211017;

    if (reg_len > BUFFER_SIZE_MAX / 2) {
        reg_len = BUFFER_SIZE_MAX / 2;
    }
    if (read_len > BUFFER_SIZE_MAX / 2) {
        read_len = BUFFER_SIZE_MAX / 2;
    }

    memset(i2c_buffer, 0, sizeof(i2c_buffer));
    memcpy((uint8_t *)i2c_buffer, (uint8_t *)reg_buf, reg_len);

#if ((I2C_MASTER_NUMBER == 9) || (I2C_SLAVE_NUMBER == 9))
    ret = hal_gpio_i2c_recv(&cfg, device_addr, i2c_buffer, reg_len, read_len, 0);
#else
#ifdef I2C_MODE_TASK
    //Task mode API,2021-03-30
#ifdef I2C_USE_SYNC
    ret = hal_i2c_recv((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, read_len, HAL_I2C_RESTART_AFTER_WRITE, 0, 0);
#else
    hal_i2c_recv((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, read_len, HAL_I2C_RESTART_AFTER_WRITE, 0, i2c_txrx_complete_handler);
    do {
        I2C_SLAVE_TRACE(0, "---wait i2c rx complete, %s:%d", __func__, __LINE__);
        if (!i2c_txrx_complete) {
            I2C_SLAVE_DELAY_MS(5);
        }
    } while (!i2c_txrx_complete);
    i2c_txrx_complete = 0;
    ret = err_code;
#endif
#else
    //simple mode API,2021-03-31
    ret = hal_i2c_simple_recv((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, i2c_buffer + reg_len, read_len);
#endif
#endif

    memcpy((uint8_t *)bufp, (uint8_t *)(i2c_buffer + reg_len), read_len);

    if (ret) {
        I2C_SLAVE_TRACE(3, "***%s:%d,return error:0x%X", __func__, __LINE__, ret);
    }
    return ret;
}

POSSIBLY_UNUSED static uint32_t i2c_write(uint8_t device_addr, uint8_t *reg_buf, uint16_t reg_len, uint8_t *bufp, uint16_t write_len)
{
    uint32_t ret = 20211017;

    if (reg_len > BUFFER_SIZE_MAX / 2) {
        reg_len = BUFFER_SIZE_MAX / 2;
    }
    if (write_len > BUFFER_SIZE_MAX / 2) {
        write_len = BUFFER_SIZE_MAX / 2;
    }

    memset(i2c_buffer, 0, sizeof(i2c_buffer));
    memcpy((uint8_t *)i2c_buffer, (uint8_t *)reg_buf, reg_len);
    memcpy((uint8_t *)(i2c_buffer + reg_len), (uint8_t *)bufp, write_len);

#if ((I2C_MASTER_NUMBER == 9) || (I2C_SLAVE_NUMBER == 9))
    ret = hal_gpio_i2c_send(&cfg, device_addr, i2c_buffer, reg_len, write_len);
#else
#ifdef I2C_MODE_TASK
    //Task mode API,2021-03-30
#ifdef I2C_USE_SYNC
    ret = hal_i2c_send((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, write_len, 0, NULL);
#else
    hal_i2c_send((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len, write_len, 0, i2c_txrx_complete_handler);
    do {
        I2C_SLAVE_TRACE(0, "---wait i2c tx complete, %s:%d", __func__, __LINE__);
        if (!i2c_txrx_complete) {
            I2C_SLAVE_DELAY_MS(5);
        }
    } while (!i2c_txrx_complete);
    i2c_txrx_complete = 0;
    ret = err_code;
#endif
#else
    //simple mode API,2021-03-31
    ret = hal_i2c_simple_send((enum HAL_I2C_ID_T)i2c_id, device_addr, i2c_buffer, reg_len + write_len);
#endif
#endif

    if (ret) {
        I2C_SLAVE_TRACE(3, "***%s:%d,return error:0x%X", __func__, __LINE__, ret);
    }
    return ret;
}
#endif

void i2c_slave_test(void)
{
    uint32_t ret;

    I2C_SLAVE_TRACE(0, "  ");
    I2C_SLAVE_TRACE(0, "------i2c slave read/write test started------");
    I2C_SLAVE_TRACE(0, "i2c slave addr:0x%x", I2C_SLAVE_ADDR);
#ifdef COMPILE_MASTER_CODE
#if defined(I2C_MODE_TASK) && defined(I2C_USE_DMA)
    I2C_SLAVE_TRACE(0, "---i2c master: task + dma mode");
#elif defined(I2C_MODE_TASK)
    I2C_SLAVE_TRACE(0, "---i2c master: task + no dma mode");
#else
    I2C_SLAVE_TRACE(0, "---i2c master: polling mode");
#endif

#else

#if defined(I2C_MODE_TASK) && defined(I2C_USE_DMA)
    I2C_SLAVE_TRACE(0, "---i2c slave: task + dma mode");
#elif defined(I2C_MODE_TASK)
    I2C_SLAVE_TRACE(0, "---i2c slave: task + no dma mode");
#else
    I2C_SLAVE_TRACE(0, "---i2c slave: polling mode");
#endif
#endif
    I2C_SLAVE_TRACE(0, "  ");

#ifdef COMPILE_MASTER_CODE
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
#ifdef I2C_MODE_DMA
    cfg.use_dma = 1;
#else
    cfg.use_dma = 0;
#endif
#ifdef I2C_USE_SYNC
    cfg.use_sync = 1;
#else
    cfg.use_sync = 0;
#endif
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

    ret = hal_i2c_open((enum HAL_I2C_ID_T)i2c_id, &cfg);
#endif

#else

    //i2c slave config
#if (I2C_SLAVE_NUMBER == 9)
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
#ifdef I2C_MODE_DMA
    cfg.use_dma = 1;
#else
    cfg.use_dma = 0;
#endif
    cfg.use_sync = 0;
    cfg.speed = 100 * 1000;
    cfg.as_master = 0;
    cfg.addr_as_slave = I2C_SLAVE_ADDR;

    switch (I2C_SLAVE_NUMBER) {
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
            ASSERT(false, "%s:%d,error i2c id:%d", __func__, __LINE__, I2C_SLAVE_NUMBER);
            break;
        }
    }

    ret = hal_i2c_open((enum HAL_I2C_ID_T)i2c_id, &cfg);
#endif
#endif  /* end of COMPILE_MASTER_CODE */

    I2C_SLAVE_TRACE(0, "  ");
    if (!ret) {
        I2C_SLAVE_TRACE(1, "i2c open success, ret   :0x%X", ret);
    } else {
        ASSERT(false, "%s:%d, i2c open fail, ret:0x%X", __func__, __LINE__, ret);
    }
#if ((I2C_MASTER_NUMBER < 9) || (I2C_SLAVE_NUMBER < 9))
    I2C_SLAVE_TRACE(1, "i2c_cfg.mode            :%d", cfg.mode);
    I2C_SLAVE_TRACE(1, "i2c_cfg.use_dma         :%d", cfg.use_dma);
    I2C_SLAVE_TRACE(1, "i2c_cfg.use_sync        :%d", cfg.use_sync);
    I2C_SLAVE_TRACE(1, "i2c_cfg.speed           :%d", cfg.speed);
    I2C_SLAVE_TRACE(1, "i2c_cfg.as_master       :%d", cfg.as_master);
    I2C_SLAVE_TRACE(1, "i2c_cfg.addr_as_slave   :0x%x", cfg.addr_as_slave);
#else
    I2C_SLAVE_TRACE(1, "i2c_cfg.scl             :%d", cfg.scl);
    I2C_SLAVE_TRACE(1, "i2c_cfg.sda             :%d", cfg.sda);
    I2C_SLAVE_TRACE(1, "i2c_cfg.speed           :%d", cfg.speed);
#endif
    I2C_SLAVE_TRACE(0, "  ");

    while (1) {
#ifdef COMPILE_MASTER_CODE
        I2C_SLAVE_TRACE(TR_ATTR_NO_LF, "i2c master write cmd:");
        I2C_SLAVE_DUMP8("%02X ", master_reg_buf, sizeof(master_reg_buf));
        I2C_SLAVE_TRACE(TR_ATTR_NO_LF, "i2c master write data:");
        I2C_SLAVE_DUMP8("%02X ", master_data_buf, sizeof(master_data_buf));
        i2c_write(I2C_SLAVE_ADDR, master_reg_buf, sizeof(master_reg_buf), master_data_buf, sizeof(master_data_buf));
#ifdef I2C_LOOP_MODE
        ret = i2c_read(I2C_SLAVE_ADDR, master_reg_buf, sizeof(master_reg_buf), slave_data_buf, sizeof(slave_data_buf));
        I2C_SLAVE_TRACE(TR_ATTR_NO_LF, "i2c master read data:");
        I2C_SLAVE_DUMP8("%02X ", slave_data_buf, sizeof(slave_data_buf));
#endif
        I2C_SLAVE_DELAY_MS(50);

#else

#ifdef I2C_MODE_TASK
        hal_i2c_slv_task_send_recv((enum HAL_I2C_ID_T)i2c_id, slave_data_buf, slave_rx_cnt, 0,
            i2c_slave_txrx_complete_handler, i2c_read_req_handler);
        I2C_SLAVE_TRACE(0, "---wait i2c rx complete, %s:%d", __func__, __LINE__);
        while (!i2c_txrx_complete);
        i2c_txrx_complete = 0;
        ret = err_code;
        if (!ret) {
            if (i2c_transport_dir == 0) {
                I2C_SLAVE_TRACE(TR_ATTR_NO_LF, "i2c slave write data:");
                I2C_SLAVE_DUMP8("%02X ", &slave_data_buf[1], slave_rx_cnt - 1);
            } else {
                I2C_SLAVE_TRACE(TR_ATTR_NO_LF, "i2c slave read data:");
                I2C_SLAVE_DUMP8("%02X ", slave_data_buf, slave_rx_cnt);
            }
#ifndef I2C_LOOP_MODE
            memset(slave_data_buf, 0, sizeof(slave_data_buf));
#endif
        }
#else
        ret = hal_i2c_slv_simple_recv((enum HAL_I2C_ID_T)i2c_id, slave_data_buf, slave_rx_cnt, 0);
#ifdef I2C_LOOP_MODE
        ret = hal_i2c_slv_simple_send((enum HAL_I2C_ID_T)i2c_id, &slave_data_buf[1], slave_rx_cnt - 1, 0);
#endif
        if (!ret) {
            I2C_SLAVE_TRACE(TR_ATTR_NO_LF, "i2c slave read data:");
            I2C_SLAVE_DUMP8("%02X ", slave_data_buf, slave_rx_cnt);
            memset(slave_data_buf, 0, sizeof(slave_data_buf));
        }
#endif
        else {
            I2C_SLAVE_TRACE(0, "i2c slave read data: ***error***, ret=0x%x", ret);
        }
#endif
    }
}

#endif

