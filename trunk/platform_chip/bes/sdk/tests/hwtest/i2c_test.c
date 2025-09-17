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
#ifdef I2C_TEST

#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_i2c.h"
#include "hwtest.h"
#include "string.h"

#define I2C_DEV_ADDR_MPU6050 (0x68 | 0)
#define I2C_DEV_ADDR_ADXL345 (0x53 | 0)
#define I2C_DEV_ADDR_BES2X00 (0x17)

#ifdef I2C_TEST_DMA_MODE
#ifndef I2C_TEST_INT_MODE
#define I2C_TEST_INT_MODE
#endif
#endif

#ifndef I2C_SPEED
#define I2C_SPEED           100000
#endif

//#define TEST_I2C_DEV_ADDR (I2C_DEV_ADDR_ADXL345)
//#define TEST_I2C_DEV_ADDR (I2C_DEV_ADDR_MPU6050)
#define TEST_I2C_DEV_ADDR (I2C_DEV_ADDR_BES2X00)

#define readl(addr) (*(volatile unsigned int *)(addr))
#define writel(val, addr) ((*(volatile unsigned int *)(addr))=(val))

struct i2c_bus;

struct i2c_dev {
    const char *name;
    enum HAL_I2C_ID_T bus;
    uint32_t addr; //device address
    int speed;
};

struct i2c_bus {
    int id;
    int inited;
    int dev_nr;
    struct i2c_dev *act;
    struct i2c_dev **dev_list;
};

static struct i2c_dev mpu6050_dev = {
    .name = "mpu6050",
    .bus = HAL_I2C_ID_0,
    .addr = I2C_DEV_ADDR_MPU6050,
    .speed = I2C_SPEED,
};

static struct i2c_dev adxl345_dev = {
    .name = "adxl345",
    .bus = HAL_I2C_ID_0,
    .addr = I2C_DEV_ADDR_ADXL345,
    .speed = I2C_SPEED,
};

static struct i2c_dev bes2x00_dev = {
    .name = "bes2x00",
    .bus = HAL_I2C_ID_0,
    .addr = I2C_DEV_ADDR_BES2X00,
    .speed = I2C_SPEED,
};


#define I2C_DEV_NR 3
#define I2C_BUS_NR 2

static struct i2c_dev *active = 0;
static struct i2c_dev *i2c_dev_list[I2C_DEV_NR] = {
    &mpu6050_dev,
    &adxl345_dev,
    &bes2x00_dev,
};

static struct i2c_bus i2c_buses[I2C_BUS_NR];

int i2c_init(int bus_id)
{
    struct i2c_bus *bus = &i2c_buses[bus_id];

    if (!bus->inited) {
        bus->id = bus_id;

        bus->act = NULL;
        bus->dev_list = i2c_dev_list;
        bus->dev_nr = I2C_DEV_NR;
        bus->inited = 1;
    }
    return 0;
}

static int i2c_match_device(uint32_t dev_addr)
{
    int i, j;

    for (i = 0;i < I2C_BUS_NR; i++) {
        struct i2c_bus *bus = &i2c_buses[i];
        struct i2c_dev *pdev;

        if (!bus->inited)
            continue;

        for(j = 0; j < bus->dev_nr; j++) {
            pdev = bus->dev_list[j];
            if (pdev->addr == dev_addr) {
                bus->act = pdev;
                active = bus->act;
                break;
            }
        }
    }

    if (!active) {
        TRACE(1,"not find dev 0x%x", dev_addr);
        return -1;
    }
    TRACE(5,"active=0x%8x, %s, %d, 0x%x, %d Hz",
            (int)active, active->name, active->bus, active->addr, active->speed);
    return 0;
}

int i2c_probe(uint32_t dev_addr)
{
    int ret;
    struct HAL_I2C_CONFIG_T cfg;

    ret = i2c_match_device(dev_addr);
    if (ret) {
        TRACE(1,"i2c match dev 0x%x failed", dev_addr);
        return -1;
    }

    memset(&cfg, 0, sizeof(cfg));
#ifdef I2C_TEST_INT_MODE
    cfg.mode = HAL_I2C_API_MODE_TASK;
#else
    cfg.mode = HAL_I2C_API_MODE_SIMPLE;
#endif
    cfg.use_dma =
#ifdef I2C_TEST_DMA_MODE
        1;
#else
        0;
#endif
    cfg.use_sync = 1;
    cfg.speed = active->speed;
    cfg.as_master = 1;

    ret = hal_i2c_open(active->bus, &cfg);
    if (ret) {
        TRACE(2,"%s, probe failed %d", __func__, ret);
        return ret;
    }
    TRACE(1,"%s, done", __func__);
    return 0;
}

int i2c_write(uint32_t dev_addr, uint8_t *reg_addr, uint32_t reg_len,
                            uint8_t *pd, uint32_t dlen)
{
    struct i2c_dev *pdev = active;
    uint8_t txbuf[32];
    uint32_t reqlen = 0;
    int ret;

    if (pdev->addr != dev_addr) {
        //TODO: probe again
        TRACE(2,"%s, dev addr 0x%x is not active", __func__, dev_addr);
        return -1;
    }

    if (reg_addr) {
        memcpy(&txbuf[reqlen], reg_addr, reg_len);
        reqlen += reg_len;
    }
    if (pd) {
        memcpy(&txbuf[reqlen], pd, dlen);
        reqlen += dlen;
    }

    TRACE(5,"%s, bus:%d, addr:0x%x, txbuf:0x%x, reqlen:%d", __func__,
            pdev->bus, pdev->addr, (int)txbuf, reqlen);

#ifdef I2C_TEST_INT_MODE
    ret = hal_i2c_task_send(pdev->bus, pdev->addr, txbuf, reqlen, 0, NULL);
#else
    ret = hal_i2c_simple_send(pdev->bus, pdev->addr, txbuf, reqlen);
#endif
    if (ret) {
        TRACE(4,"%s, send failed, ret=0x%x, reg_len=%d, dlen=%d",
                __func__, ret, reg_len, dlen);
    }
    TRACE(1,"%s, done", __func__);
    return ret;
}

int i2c_read(uint32_t dev_addr, uint8_t *reg_addr, uint32_t reg_len,
                            uint8_t *pd, uint32_t dlen)
{
    struct i2c_dev *pdev = active;
    uint8_t buf[32];
    uint32_t reqlen = 0;
    int ret;

    TRACE(6,"%s, dev_addr=0x%x, reg_addr=0x%x, reg_len=%d, pd=0x%x, dlen=%d",
        __func__, dev_addr, (uint32_t)reg_addr, (uint32_t)reg_len, (uint32_t)pd, dlen);

    if (pdev->addr != dev_addr) {
        //TODO: probe again
        TRACE(2,"%s, dev addr 0x%x is not active", __func__, dev_addr);
        return -1;
    }

    if (reg_addr) {
        memcpy(&buf[reqlen], reg_addr, reg_len);
        reqlen += reg_len;
    }
#ifdef I2C_TEST_INT_MODE
    ret = hal_i2c_task_recv(pdev->bus, pdev->addr, buf, reqlen, buf + reqlen, dlen, 0, NULL);
#else
    ret = hal_i2c_simple_recv(pdev->bus, pdev->addr, buf, reqlen, buf + reqlen, dlen);
#endif
    if (ret) {
        TRACE(4,"%s, read failed, ret=0x%x, reqlen=%d, dlen=%d",
                __func__, ret, reqlen, dlen);
        goto exit;
    }
    if(pd) {
        memcpy(pd, &buf[reqlen], dlen);
    }

exit:
    TRACE(1,"%s, done", __func__);
    return ret;
}

#ifdef I2C_TEST_INT_MODE
int i2c_mread(uint32_t dev_addr, uint8_t *reg_addr, uint32_t reg_item_len,
                            uint8_t *pd, uint32_t data_item_len, uint32_t item_cnt)
{
    struct i2c_dev *pdev = active;
    int ret;

    TRACE(5,"%s: dev_addr=0x%x reg_item_len=%d data_item_len=%d item_cnt=%d",
        __func__, dev_addr, reg_item_len, data_item_len, item_cnt);

    if (pdev->addr != dev_addr) {
        //TODO: probe again
        TRACE(2,"%s, dev addr 0x%x is not active", __func__, dev_addr);
        return -1;
    }

    ret = hal_i2c_task_mrecv(pdev->bus, pdev->addr, reg_addr, reg_item_len, pd, data_item_len, item_cnt, 0, NULL);
    if (ret) {
        TRACE(2,"%s, read failed, ret=0x%x", __func__, ret);
        goto exit;
    }

exit:
    TRACE(1,"%s, done", __func__);
    return ret;
}
#endif

int i2c_close(uint32_t dev_addr)
{
    hal_i2c_close(HAL_I2C_ID_0);
    return 0;
}

int i2c_bare_rw(int id)
{
    uint32_t base = I2C0_BASE, len;
    uint32_t rdata;
    uint32_t dev_id = 0x17;

#ifdef I2C1_BASE
    if (id == 0)
        base = I2C0_BASE;
    else if (id == 1)
        base = I2C1_BASE;
#endif

    TRACE(3,"%s i2c-%d, dev_id=0x%x", __func__, id, dev_id);

    writel(0x00, base + 0x6c); //disable i2c

    writel(dev_id, base + 0x04); //tar

    writel(0x80, base + 0x14); //hcnt
    writel(0x90, base + 0x18); //lcnt

    writel(0x63, base + 0x00);//restart_en, slave disable

    writel(0x01, base + 0x6c); //enable i2c

    writel(0x00, base + 0x10); //register addr 0x00

    writel(0x100, base + 0x10); //read=1,
    writel(0x300, base + 0x10); //stop=1, read=1

    len = 2;

    do {
        if (readl(base + 0x70) & (1<<3)) {
            rdata = readl(base + 0x10);
            TRACE(2,"%s, read data=0x%x", __func__, rdata);
            len--;
        }
        if (len == 0)
            break;
    } while(1);

    return 0;
}

int do_i2c_test(void)
{
    int ret = 0;
    uint32_t dev_addr = TEST_I2C_DEV_ADDR;
    uint32_t reg, val, vlen = 1;

#ifdef I2C_TEST_INT_MODE
    TRACE(0,"test i2c interrupt mode");
#else
    TRACE(0,"test i2c polling mode");
#endif

    ret = i2c_init(HAL_I2C_ID_0);
    if (ret) {
        TRACE(2,"%s, init failed, %d", __func__, ret);
    }

    ret = i2c_probe(dev_addr);
    if (ret) {
        TRACE(2,"%s, probe failed, %d", __func__, ret);
        goto exit;
    }

#if 0
    i2c_bare_rw(0);
    while(1);
#endif

    // read device id register
    switch(dev_addr) {
    case I2C_DEV_ADDR_ADXL345:
        reg = 0x00;
        vlen = 1;
        break;
    case I2C_DEV_ADDR_MPU6050:
        reg = 0x75;
        vlen = 1;
        break;
    case I2C_DEV_ADDR_BES2X00:
        reg = 0x00;
        vlen = 2;
        break;
    default:
        reg = 0x01;
        vlen = 1;
        break;
    }
    val = 0;
    ret = i2c_read(dev_addr, (uint8_t *)&reg, 1, (uint8_t *)&val, vlen);
    if (ret) {
        TRACE(3,"%s: read reg 0x%x failed, 0x%x", __func__, reg, ret);
        goto exit;
    }
    if (vlen == 2) {
        val = ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
    }
    TRACE(2,"read ID reg 0x%02x = 0x%04x", reg, val);

#ifdef I2C_TEST_INT_MODE
    uint8_t reg_array[3];
    uint16_t value_array[ARRAY_SIZE(reg_array)];
    uint32_t i;
    for (i = 0; i < ARRAY_SIZE(reg_array); i++) {
        reg_array[i] = i;
    }
    ret = i2c_mread(dev_addr, &reg_array[0], 1, (uint8_t *)&value_array[0], vlen, ARRAY_SIZE(reg_array));
    if (ret) {
        TRACE(2,"%s: mread reg failed ret=0x%x", __func__, ret);
        goto exit;
    }
    for (i = 0; i < ARRAY_SIZE(reg_array); i++) {
        if (vlen == 2) {
            value_array[i] = ((value_array[i] & 0xFF) << 8) | ((value_array[i] >> 8) & 0xFF);
        }
        TRACE(3,"read reg array[%d] 0x%02x = 0x%04x", i, reg_array[i], value_array[i]);
    }
#endif

#if 0
    // read rw register
    switch(dev_addr) {
    case I2C_DEV_ADDR_ADXL345:
        reg = 0x1d;
        vlen = 1;
        break;
    case I2C_DEV_ADDR_MPU6050:
        reg = 0x23;
        vlen = 1;
        break;
    case I2C_DEV_ADDR_BES2X00:
    default:
        reg = 0x01;
        vlen = 2;
        break;
    }
    val = 0;
    ret = i2c_read(dev_addr, (uint8_t *)&reg, 1, (uint8_t *)&val, vlen);
    if (ret) {
        TRACE(3,"%s, read reg 0x%x failed, %d", __func__, reg, ret);
        goto exit;
    }
    TRACE(2,"read reg 0x%x = 0x%x", reg, val);

    if (val & 0x80) {
        val &= ~0x80;
    } else {
        val |= 0x80;
    }

    ret = i2c_write(dev_addr, (uint8_t *)&reg, 1, (uint8_t *)&val, vlen);
    if (ret) {
        TRACE(3,"%s, write reg 0x%x failed, %d", __func__, reg, ret);
        goto exit;
    }
    TRACE(2,"write reg 0x%x = 0x%x", reg, val);

    ret = i2c_read(dev_addr, (uint8_t *)&reg, 1, (uint8_t *)&val, vlen);
    if (ret) {
        TRACE(3,"%s, read reg 0x%x failed, %d", __func__, reg, ret);
        goto exit;
    }
    TRACE(2,"read reg 0x%x = 0x%x", reg, val);
#endif

exit:
    i2c_close(dev_addr);
    TRACE(1,"%s, done", __func__);
    return ret;
}

void i2c_test(void)
{
    int r;

    r = do_i2c_test();
    if (r)
        TRACE(2,"%s error ### 0x%x",__func__, r);
    else
        TRACE(1,"%s okay", __func__);
}

#endif
