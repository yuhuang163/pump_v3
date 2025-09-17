/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifdef UTILS_ESHELL_EN
#if defined(__SYS_AS_MAIN__)

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis_os.h"
#include "math.h"
#include "hal_iomux.h"
#include "hal_i2c.h"
#include "hal_trace.h"
#include "eshell.h"

#define I2C_TEST_SIZE      8
#define TEST_I2C_SLAVE_ADDR     0x66

static uint8_t master_tx_buff[I2C_TEST_SIZE];
static uint8_t slave_rx_buff[I2C_TEST_SIZE];

static void test_cmd_usage(void)
{
    eshell_putstring("Usage\r\n");
    eshell_putstring("utest_i2c read bus(0/1) mode(task:0/simple:1) dev_addr(hex) reg_addr(hex) reg_len rx_len\r\n");
    eshell_putstring("utest_i2c write bus(0/1) mode(task:0/simple:1) dev_addr(hex) reg_addr(hex) reg_len txdata0(hex) txdata1(hex) txdata2(hex) ... \r\n");
}

static uint32_t test_i2c_open(enum HAL_I2C_ID_T id, enum HAL_I2C_API_MODE_T mode)
{
    struct HAL_I2C_CONFIG_T i2c_cfg;
    memset(&i2c_cfg, 0, sizeof(i2c_cfg));

    i2c_cfg.speed = 400000;
    i2c_cfg.as_master = 1;
    i2c_cfg.addr_as_slave = 0;
    i2c_cfg.rising_time_ns = 0;

    if (mode == HAL_I2C_API_MODE_SIMPLE){
        i2c_cfg.mode = HAL_I2C_API_MODE_SIMPLE;
        i2c_cfg.use_dma = 0;
        i2c_cfg.use_sync = 0;
    } else if (mode == HAL_I2C_API_MODE_TASK){
        i2c_cfg.mode = HAL_I2C_API_MODE_TASK;
        i2c_cfg.use_dma = 1;
        i2c_cfg.use_sync = 1;
    }

    return hal_i2c_open(id, &i2c_cfg);
}

static void test_i2c_read(enum HAL_I2C_ID_T id, enum HAL_I2C_API_MODE_T mode, uint8_t dev_addr, uint16_t reg_buf, uint16_t reg_len, uint16_t rx_len)
{
    uint32_t ret = 0;
    uint8_t *pdata = NULL;
    pdata = (uint8_t *)malloc(rx_len);
    if (!pdata) {
        return;
    }

    ret = test_i2c_open(id, mode);
    if(ret != 0){
        free(pdata);
        eshell_putstring("i2c_open fail ret=%#x\r\n", ret);
        return;
    }

    if (mode == HAL_I2C_API_MODE_SIMPLE){
        ret = hal_i2c_simple_recv(id, dev_addr, (uint8_t *)&reg_buf, reg_len, pdata, rx_len);
    } else if (mode == HAL_I2C_API_MODE_TASK){
        ret = hal_i2c_task_recv(id, dev_addr, (uint8_t *)&reg_buf, reg_len, pdata, rx_len, 0, NULL);
    }

    if (!ret) {
        for (int i = 0; i < rx_len; i++) {
            eshell_putstring("data[%d] = %#x\r\n", i, pdata[i]);
        }
    } else {
        eshell_putstring("i2c recv failed id:%d ret=%08x\r\n", id, ret);
    }

    free(pdata);
    hal_i2c_close(id);
}

static void test_i2c_write(enum HAL_I2C_ID_T id, enum HAL_I2C_API_MODE_T mode, uint8_t dev_addr, uint16_t reg_addr, uint8_t *pdata, uint16_t reg_len, uint16_t tx_len)
{
    uint32_t ret = 0;
    uint8_t *i2c_x_tx_buf = NULL;
    i2c_x_tx_buf = (uint8_t *)malloc(reg_len+tx_len);
    if (!i2c_x_tx_buf) {
        return;
    }

    ret = test_i2c_open(id, mode);
    if(ret != 0){
        free(i2c_x_tx_buf);
        eshell_putstring("i2c_open fail ret=%#x\r\n", ret);
        return;
    }

    if (reg_len) {
        memcpy(&i2c_x_tx_buf[0], &reg_addr, reg_len);
    }
    if (tx_len) {
        memcpy(&i2c_x_tx_buf[reg_len], pdata, tx_len);
    }

    if (mode == HAL_I2C_API_MODE_SIMPLE){
        ret = hal_i2c_simple_send(id, dev_addr, i2c_x_tx_buf, reg_len + tx_len);
    } else if (mode == HAL_I2C_API_MODE_TASK){
        ret = hal_i2c_send(id, dev_addr, i2c_x_tx_buf, reg_len, tx_len, 0, NULL);
    }

    if (ret != 0) {
        eshell_putstring("i2c simple send failed id:%d ret=%08x\r\n", id, ret);
    }

    free(i2c_x_tx_buf);
    hal_i2c_close(id);
}


static void test_i2c_master_tx_loop(enum HAL_I2C_ID_T id, uint32_t speed)
{
    eshell_putstring("%s\r\n", __func__);
    int ret;
    struct HAL_I2C_CONFIG_T i2c_cfg;
    memset(&i2c_cfg, 0, sizeof(i2c_cfg));
    if(id == 0)
    {
        hal_iomux_set_i2c0();
    }
    else
    {
        hal_iomux_set_i2c1();
    }
    i2c_cfg.as_master = 1; //1 - master, 0 - slave
    i2c_cfg.mode = HAL_I2C_API_MODE_SIMPLE;
    i2c_cfg.speed = speed;
    ret = hal_i2c_open(id, &i2c_cfg);
    ASSERT(ret ==0, "master open i2c fail");

    for (uint32_t i = 0; i < sizeof(master_tx_buff); i++){
         master_tx_buff[i] = rand() & 0xff;
    }

    while(1)
    {
        ret = hal_i2c_simple_send(id, TEST_I2C_SLAVE_ADDR, master_tx_buff, sizeof(master_tx_buff));
        eshell_putstring("i2c master write data: %d\r\n", ret);
        osDelay(50);
    }
}


static void test_i2c_slave_rx_loop(enum HAL_I2C_ID_T id, uint32_t speed)
{
    eshell_putstring("%s\r\n", __func__);
    int ret;
    if(id == 0)
    {
        hal_iomux_set_i2c0();
    }
    else
    {
        hal_iomux_set_i2c1();
    }
    struct HAL_I2C_CONFIG_T i2c_cfg;
    memset(&i2c_cfg, 0, sizeof(i2c_cfg));

    i2c_cfg.mode = HAL_I2C_API_MODE_SIMPLE;

    i2c_cfg.use_dma = 1;
    i2c_cfg.use_sync = 0;
    i2c_cfg.speed = speed;
    i2c_cfg.as_master = 0;
    i2c_cfg.addr_as_slave = TEST_I2C_SLAVE_ADDR;
    i2c_cfg.rising_time_ns = 0;

    ret = hal_i2c_open(id, &i2c_cfg);
    ASSERT(ret ==0, "slave open i2c fail");
    while(1)
    {
        ret = hal_i2c_slv_simple_recv(id, slave_rx_buff, sizeof(slave_rx_buff), 0);
        eshell_putstring("i2c slave receive data: %d\r\n", ret);
        //osDelay(1000);
    }
}


static void unitest_i2c(int argc, char *argv[])
{
     if (strncmp(argv[1], "loop", 4) == 0)
     {
        int i2c_role = atoi(argv[2]);
        enum HAL_I2C_ID_T i2c_id = atoi(argv[3]);
        uint32_t i2c_speed = atoi(argv[4]);
        if ((i2c_id < 0) || (i2c_id > 1))
        {
            eshell_putstring("i2c_id err, valid value: 0 or 1, 0:i2c0   1:i2c1\r\n");
            return;
        }
        if(1 == i2c_role)
        {
            test_i2c_master_tx_loop(i2c_id, i2c_speed);
        }
        else
        {
            test_i2c_slave_rx_loop(i2c_id, i2c_speed);
        }
        return;
     }
    

    if (argc < 7) {
        test_cmd_usage();
        return;
    }

    /**** i2c_id valid value: 0 or 1, 0:i2c0 1:i2c1*****/
    enum HAL_I2C_ID_T i2c_id = atoi(argv[2]);
    if ((i2c_id < 0) || (i2c_id > 1)){
        eshell_putstring("i2c_id err, valid value: 0 or 1, 0:i2c0   1:i2c1\r\n");
        return;
    }

    /**** i2c_mode valid value: 0 or 1, 0:task_mode 1:simple_mode ****/
    enum HAL_I2C_API_MODE_T i2c_mode;
    i2c_mode = atoi(argv[3]);
    if ((i2c_mode < 0) || (i2c_mode > 1)){
        eshell_putstring("i2c_mode err, valid value: 0 or 1, 0:task_mode 1:simple_mode\r\n");
        return;
    }

    /**** dev_addr, reg_addr, reg_len*****/
    unsigned int dev_addr, reg_addr;
    sscanf(argv[4], "%x", &dev_addr);
    sscanf(argv[5], "%x", &reg_addr);
    if (reg_addr > 0xFF) {
        reg_addr = ((reg_addr >> 8) & 0x00FF) | ((reg_addr << 8) & 0xFF00);
    }
    uint16_t reg_len = atoi(argv[6]);
    eshell_putstring("i2c_mode=%d dev_addr=%#x reg_addr=%#x reg_len=%d \r\n", i2c_mode, dev_addr, reg_addr, reg_len);

    /**** i2c_cmd valid value: read or write ****/
    if (strncmp(argv[1], "read", 4) == 0) { // read
        uint16_t rx_len = atoi(argv[7]);
        test_i2c_read(i2c_id, i2c_mode, dev_addr, reg_addr, reg_len, rx_len);
    } else if (strncmp(argv[1], "write", 5) == 0) { // write
        uint16_t tx_len = argc - 7;
        uint8_t *pdata = NULL;
        pdata = (uint8_t *)malloc(tx_len);
        if (!pdata) {
            eshell_putstring("memory malloc fail\r\n");
            return;
        }
        unsigned int tx_data;
        for (int i = 0; i < tx_len; i++) {
            sscanf(argv[i + 7], "%x", &tx_data);
            pdata[i] = (uint8_t)tx_data;
            eshell_putstring("data[%d]=%#x\r\n", i, pdata[i]);
        }
        test_i2c_write(i2c_id, i2c_mode, dev_addr, reg_addr, pdata, reg_len, tx_len);
        free(pdata);
    } else {
        test_cmd_usage();
    }

}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_i2c", "usage: utest_i2c help", unitest_i2c);

#endif // __SYS_AS_MAIN__
#endif // UTILS_ESHELL_EN
