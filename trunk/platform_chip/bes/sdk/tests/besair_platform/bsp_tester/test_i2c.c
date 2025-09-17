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
#ifdef TEST_I2C
#include "string.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_psc.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_i2c.h"

#define I2C_CLK_FREQ            (4 * 100 * 1000)

static int test_i2c_deinit(enum HAL_I2C_ID_T i2c_id);

static int test_i2c_iomux_init(enum HAL_I2C_ID_T i2c_id)
{
    TRACE(1, "%s id:%d", __func__, (int)i2c_id);

    if (i2c_id == HAL_I2C_ID_0) {
        hal_iomux_set_i2c0();
#ifdef I2C1_BASE
    } else if (i2c_id == HAL_I2C_ID_1) {
        hal_iomux_set_i2c1();
#endif
#ifdef I2C2_BASE
    } else if (i2c_id == HAL_I2C_ID_2) {
        hal_iomux_set_sens_i2c2();
#endif
#ifdef I2C3_BASE
    } else if (i2c_id == HAL_I2C_ID_3) {
        hal_iomux_set_sens_i2c3();
#endif
#ifdef I2C4_BASE
    } else if (i2c_id == HAL_I2C_ID_4) {
        hal_iomux_set_sens_i2c4();
#endif
    }
    return 0;
}

// I2C Initialization.
// i2c_id: i2c id.
// cfg: i2c config.
static int test_i2c_init(enum HAL_I2C_ID_T i2c_id, struct HAL_I2C_CONFIG_T *cfg)
{
    int ret = 0;
#ifdef I2C2_BASE
    if (i2c_id == HAL_I2C_ID_2
#ifdef I2C3_BASE
            || i2c_id == HAL_I2C_ID_3
#endif
            ){
#ifndef CHIP_SUBSYS_SENS
        hal_psc_sens_enable();
        hal_cmu_sens_clock_enable();
        hal_cmu_sens_reset_clear();
        __DSB();
        osDelay(1);
#endif
    }
#endif
    // I2C open.
    ret = (int)hal_i2c_open(i2c_id, cfg);
    if (ret) {
        TRACE(0,"%s hal_i2c_open failed, id:%d ret = %d.", __func__, (int)i2c_id, ret);
        test_i2c_deinit(i2c_id);
    }

    return ret;
}

static int test_i2c_deinit(enum HAL_I2C_ID_T i2c_id)
{
    hal_i2c_close(i2c_id);
#ifdef I2C2_BASE
    if (i2c_id == HAL_I2C_ID_2 || i2c_id == HAL_I2C_ID_3){
#ifndef CHIP_SUBSYS_SENS
        hal_cmu_sens_reset_set();
        hal_cmu_sens_clock_disable();
        hal_psc_sens_disable();
#endif
    }
#endif
    return 0;
}

void test_i2c(void)
{
    int ret = 0;
    uint8_t i2c_tx_buf[3] = {0x00};
    uint16_t reg_data;
    struct HAL_I2C_CONFIG_T i2c_cfg;
    enum HAL_I2C_ID_T i2c_id;

    while(1){
        TRACE_OUTPUT_LINEFEED();
        TRACE_IMM(0, "###############     hal_i2c_simple test begin      ###############");
        for (uint32_t i = 0; i < HAL_I2C_ID_NUM; i++){
            memset(&i2c_cfg, 0, sizeof(i2c_cfg));
            i2c_id = (enum HAL_I2C_ID_T)i;
            i2c_cfg.mode            = HAL_I2C_API_MODE_SIMPLE;
            // i2c_cfg.use_dma         = 1;         //un use
            // i2c_cfg.use_sync        = 1;         //un use
            i2c_cfg.speed           = I2C_CLK_FREQ;
            i2c_cfg.as_master       = 1;
            // i2c_cfg.addr_as_slave   = 0;         //un use
            test_i2c_iomux_init(i2c_id);
            test_i2c_init(i2c_id, &i2c_cfg);
            reg_data = 0;
            ret = hal_i2c_simple_recv(i2c_id, 0x27, i2c_tx_buf, 1, (uint8_t *)&reg_data, 2);
            if (ret){
                TRACE(0, "%s mst_write failed id:%d ret=%08x", __func__, (int)i2c_id, ret);
            }
            TRACE(0,"hal_i2c_simple_recv reg_data[0x%04x]", reg_data);
            test_i2c_deinit(i2c_id);
            osDelay(500);
        }

        TRACE_OUTPUT_LINEFEED();
        TRACE_IMM(0, "###############     hal_i2c_task test begin       ###############");
        for (uint32_t i = 0; i < HAL_I2C_ID_NUM; i++){
            memset(&i2c_cfg, 0, sizeof(i2c_cfg));
            i2c_id = (enum HAL_I2C_ID_T)i;
            i2c_cfg.mode            = HAL_I2C_API_MODE_TASK;
            i2c_cfg.use_dma         = 1;
            i2c_cfg.use_sync        = 1;
            i2c_cfg.speed           = I2C_CLK_FREQ;
            i2c_cfg.as_master       = 1;
#ifdef CHIP_BEST1600
#ifdef CHIP_SUBSYS_SENS
            if((HAL_I2C_ID_3 == i || HAL_I2C_ID_4 == i) && 1 == i2c_cfg.use_dma) {
                TRACE(0, "1600 sensor_hub i2c ch%d does not support dma, skip it", i);
                continue;
            }
            if(HAL_I2C_ID_4 == i) {
                TRACE(0, "1600 sensor_hub i2c ch%d does not support task mode, skip it", i);
                continue;
            }
#elif defined(BTH_AS_MAIN_MCU)
            i2c_cfg.use_dma         = 0;
            TRACE(0, "1600 bth_as_main i2c ch%d does not support dma, test no dma mode", i);
#endif
#endif
            test_i2c_iomux_init(i2c_id);
            test_i2c_init(i2c_id, &i2c_cfg);
            i2c_tx_buf[1] = 0;
            i2c_tx_buf[2] = 0;
            ret = hal_i2c_recv(i2c_id, 0x27, i2c_tx_buf, 1, 2, HAL_I2C_STOP_AFTER_WRITE, 0, 0);
            if (ret){
                TRACE(0, "%s i2c task recv failed id:%d ret=%08x", __func__, (int)i2c_id, ret);
            }
            TRACE(0,"hal_i2c_task_recv reg_data[0x%02x%02x]", i2c_tx_buf[1], i2c_tx_buf[2]);
            test_i2c_deinit(i2c_id);
            osDelay(500);
        }
    }
}
#endif


