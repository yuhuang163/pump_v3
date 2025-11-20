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
#ifdef CHIP_FUNCTION_TEST_I2C

#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_i2c.h"
#include "string.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_timer.h"
#include "hal_sleep.h"
#include "pmu.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_sysfreq.h"

#ifndef I2C_TEST_TASK_MODE
//#define I2C_TEST_TASK_MODE
#endif
#define I2C_TEST_ADDR_BES2X00 (0x11)

#define I2C_TEST_DEV_ADDR      (I2C_DEV_ADDR_BES2X00)
#ifndef I2C_TEST_I2C_ID
//#define I2C_TEST_I2C_ID        1
#err undifine I2C_TEST_I2C_ID
#endif
#define I2C_TEST_I2C_SPEED     (100)*(1000)*1
#define I2C_TEST_ADDR          I2C_TEST_ADDR_BES2X00
#define I2C_TEST_KEEY_TIME_MS  10000
//static uint32_t i2c_test_speed = I2C_TEST_I2C_SPEED;
static int _i2c_test_init(int id, uint32_t i2c_speed)
{
    struct HAL_I2C_CONFIG_T cfg;
    int ret;

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
    cfg.speed = i2c_speed;
    cfg.as_master = 1;

    ret = hal_i2c_open(id, &cfg);
    if (ret) {
        TRACE(2,"%s, i2c open(%d) fail ret = %d", __func__, id, ret);
        return ret;
    }
    TRACE(1,"%s, done", __func__);
    return 0;
}

static void _i2c_close(int id)
{
    hal_i2c_close(id);
    TRACE(1,"%s, done", __func__);
}

int _i2c_test_write(uint32_t dev_addr, uint8_t *reg_addr, uint32_t reg_len,
                            uint8_t *pd, uint32_t dlen)
{
    uint8_t txbuf[32];
    uint32_t reqlen = 0;
    int ret;
    enum HAL_I2C_ID_T id = I2C_TEST_I2C_ID;

    if (I2C_TEST_ADDR != dev_addr) {
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

    //TRACE(5,"%s, id:%d, dev_addr:0x%x, txbuf:0x%x, reqlen:%d", __func__,
    //        id, dev_addr, (int)txbuf, reqlen);

#ifdef I2C_TEST_TASK_MODE
    ret = hal_i2c_task_send(id, dev_addr, txbuf, reqlen, 0, NULL);
#else
    ret = hal_i2c_simple_send(id, dev_addr, txbuf, reqlen);
#endif
    if (ret) {
        TRACE(4,"%s, send failed, ret=0x%x, reg_len=%d, dlen=%d",
                __func__, ret, reg_len, dlen);
    }
    //TRACE(1,"%s, done", __func__);
    return ret;
}

int _i2c_test_read(uint32_t dev_addr, uint8_t *reg_addr, uint32_t reg_len,
                            uint8_t *pd, uint32_t dlen)
{
    uint8_t buf[32];
    uint32_t reqlen = 0;
    int ret;
    enum HAL_I2C_ID_T id = I2C_TEST_I2C_ID;

   // TRACE(6,"%s, dev_addr=0x%x, reg_addr=0x%x, reg_len=%d, pd=0x%x, dlen=%d",
    //    __func__, dev_addr, (uint32_t)reg_addr, (uint32_t)reg_len, (uint32_t)pd, dlen);

    if (I2C_TEST_ADDR != dev_addr) {
        //TODO: probe again
        TRACE(2,"%s, dev addr 0x%x is not active", __func__, dev_addr);
        return -1;
    }

    if (reg_addr) {
        memcpy(&buf[reqlen], reg_addr, reg_len);
        reqlen += reg_len;
    }
#ifdef I2C_TEST_TASK_MODE
    ret = hal_i2c_task_recv(id, dev_addr, buf, reqlen, buf + reqlen, dlen, 0, NULL);
#else
    ret = hal_i2c_simple_recv(id, dev_addr, buf, reqlen, buf + reqlen, dlen);
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
    //TRACE(1,"%s, done", __func__);
    return ret;
}

int _i2c_test_do(void)
{
    int ret;
    uint32_t reg;
    uint8_t reg_len;
    uint32_t val;
    uint32_t i2c_speed = I2C_TEST_I2C_SPEED;
    uint32_t start_time;
    uint32_t end_time;
    bool is_open = false;
    uint32_t multi = 1;

    start_time = hal_sys_timer_get();
    do{
        if(!is_open)
        {
            i2c_speed = I2C_TEST_I2C_SPEED*multi++;
            TRACE(1, "%s: I2C SPEED = %08d", __func__, i2c_speed);
            ret = _i2c_test_init(I2C_TEST_I2C_ID, i2c_speed);
            if(ret)
            {
                TRACE(1,"%s, _i2c_test_init failed, ret = %d", __func__, ret);
                break;
            }
            is_open = true;
        }
        reg = 0X4008001C;
        reg_len = 4;
        val = 0x0;
        _i2c_test_read(I2C_TEST_ADDR, (uint8_t *)&reg, reg_len, (uint8_t *)&val, 1);
        //TRACE(1, "reg: 0x%x, val = 0x%x", reg, val);
        end_time = hal_sys_timer_get();
        if(TICKS_TO_MS(end_time - start_time) > I2C_TEST_KEEY_TIME_MS)
        {
            _i2c_close(I2C_TEST_I2C_ID);
            is_open = false;
            start_time = end_time;
        }
    }while(1);

    return ret;
}

void function_test_i2c(void)
{
    int r;

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_9, HAL_CMU_FREQ_208M);
    TRACE(1, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));
    r = _i2c_test_do();
    if (r)
        TRACE(2,"%s error ### 0x%x",__func__, r);
    else
        TRACE(1,"%s okay", __func__);
}

#endif

