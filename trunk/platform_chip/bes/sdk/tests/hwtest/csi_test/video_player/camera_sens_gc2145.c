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
#ifdef CSI_SENS_GC2145
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "plat_addr_map.h"
#include "hal_location.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_i2c.h"
#include "hal_dsi.h"
#include "hal_lcdc.h"
#include "hal_csi.h"
#include "csi_dsi_conf.h"
#include "video_player.h"
#include "camera_drv.h"

#ifdef CSI_SENS_AEC_ENABLE
#define AEC_ENABLE 1
#else
#define AEC_ENABLE 0
#endif

#define CROP_ENABLE 1

#define GC2145_OP_FLAG CAMERA_OP_FLAG_CALIB_PHY

#ifndef GC2145_IMAGE_WIDTH
#define GC2145_IMAGE_WIDTH 800
#endif

#ifndef GC2145_IMAGE_HEIGHT
#define GC2145_IMAGE_HEIGHT 600
#endif

//#define CSI_SENS_I2C_DEBUG

#define I2C_USER_GC2145  (CAMERA_I2C_USER_0)
#define I2C_ID_GC2145    (CAMERA_I2C_ID_0)
#define I2C_SPEED_GC2145 (CAMERA_I2C_SPEED_200KHZ)
#define I2C_ADDR_GC2145  (0x3C)

static bool sens_pwr_status = false;

static enum VP_DEVICE_T sens_dev_type = VP_DEVICE_GC2145;
static enum VP_DEVICE_ID_T sens_dev_id = VP_DEVICE_ID_0;
static enum CAMERA_I2C_USER_T sens_i2c_user = I2C_USER_GC2145;
static enum CAMERA_I2C_ID_T   sens_i2c_id = I2C_ID_GC2145;
static uint8_t sens_i2c_addr = I2C_ADDR_GC2145;
static uint32_t sens_i2c_speed = I2C_SPEED_GC2145;

static struct VP_DEVICE_CONFIG_T sens_dev_cfg = {
    .width = GC2145_IMAGE_WIDTH,
    .height = GC2145_IMAGE_HEIGHT,
    .fmt = VP_PIXEL_FMT_YUV422,
    .channel_num = VP_CHANNEL_NUM_2,
    .op_flag = GC2145_OP_FLAG,
};

struct IMG_SENS_REG_T {
    uint8_t addr;
    uint8_t val;
};

static struct IMG_SENS_REG_T sens_reg_clk_cfg[] = {
    {0xfe, 0xf0}, //RESET:[7]soft rst, [6]cm_rst, [5]mipi_rst, [4]CISCTL_restart_n
    {0xfe, 0xf0}, //RESET
    {0xfe, 0xf0}, //RESET
    {0xfc, 0x06}, //analog_pwc  : [2]vpll_en, [1]vpix_en,[0]analog pwd enable
    {0xf6, 0x00}, //pull up/down: [5:4]up_dn, =00,not pull, [0]PWD dn, 0:pull down
    {0xf7, 0x1d}, //PLL_mode1   : [6:4]ser_clk_double, [3]clk_double, [2]NA, [1]div2_en [0]pll_en
    {0xf8, 0x84}, //PLL_mode2   : 0x84, 24->240; 0x86:24->336
    {0xfa, 0x00}, //clk_div_mode: [7:4]divide_by, [3:0] clock duty
    {0xf9, 0x8e}, //cm_mode     : clk enable cntl
    {0xf2, 0x00}, //pad io      : [4]auto vb pad hiz, [3]data_pad_io, [2:0]sync_pad_io, 0=input,1=output
};

static int i2c_read(uint8_t reg_addr, uint8_t *val)
{
    int ret;
    uint8_t buf[2] = {0};

    buf[0] = reg_addr;
    ret = camera_device_hw_i2c_recv(sens_i2c_user, sens_i2c_id, sens_i2c_addr, buf, 1, buf + 1, 1);
    if (ret) {
        TRACE(0, "[%s]: WARNING: i2c_read error, ret=%d, addr=%x", __func__, ret, reg_addr);
    }
    if(val) {
        *val = buf[1];
    }

#ifdef CSI_SENS_I2C_DEBUG
    TRACE(0, "I2C_RD:[%02x]=%02x", reg_addr, buf[1]);
#endif

    return 0;
}

static int i2c_write(uint8_t reg_addr, uint8_t val)
{
    int ret;
    uint8_t buf[2];

#ifdef CSI_SENS_I2C_DEBUG
    TRACE(0, "I2C_WR:[%02x]=%02x", reg_addr, val);
#endif

    buf[0] = reg_addr;
    buf[1] = val;
    ret = camera_device_hw_i2c_send(sens_i2c_user, sens_i2c_id, sens_i2c_addr, buf, 2);
    if (ret) {
        TRACE(0, "[%s]: WARNING: i2c_write error, ret=%d, addr=%x, val=%x", __func__, ret, reg_addr, val);
    }
    return 0;
}

static int sensor_bus_read(uint32_t reg_addr, uint32_t *val)
{
    return i2c_read((uint8_t)reg_addr, (uint8_t *)val);
}

static int sensor_bus_write(uint32_t reg_addr, uint32_t val)
{
    return i2c_write((uint8_t)reg_addr, (uint8_t)val);
}

POSSIBLY_UNUSED static int sensor_dbg_mode_set_fix_value(uint8_t v)
{
    uint8_t val;

    val = (((v) & 0xF) << 4) | 0x09;
    return i2c_write(0x8d, val);
}

POSSIBLY_UNUSED static void sensor_reg_page(int page)
{
    i2c_write(0xFE, page&0xF);
}

POSSIBLY_UNUSED static void sensor_clock_init(void)
{
    uint32_t i;
    struct IMG_SENS_REG_T *regs = sens_reg_clk_cfg;
    uint32_t num = ARRAY_SIZE(sens_reg_clk_cfg);

    for (i = 0; i < num; i++) {
        i2c_write(regs[i].addr, regs[i].val);
    }
}

POSSIBLY_UNUSED static int sensor_img_set_exposure(uint32_t exp)
{
    uint8_t val[2];

    val[0] = (exp>>8)&0xFF;
    val[1] = (exp>>0)&0xFF;
    i2c_write(0x03, val[0]);
    i2c_write(0x04, val[1]);

    return 0;
}

static void sensor_common_init(void)
{
#if 0
    i2c_write(0xfe, 0xf0);
    i2c_write(0xfe, 0xf0);
    i2c_write(0xfe, 0xf0);
    i2c_write(0xfc, 0x06);
    i2c_write(0xf6, 0x00);
    i2c_write(0xf7, 0x1d); //[1]: clk div2_en
    i2c_write(0xf8, 0x84); //0x84, 24->240; 0x86:24->336
    i2c_write(0xfa, 0x00);
    i2c_write(0xf9, 0x8e);
    i2c_write(0xf2, 0x00);
#endif
    /////////////////////////////////////////////////
    //////////////////ISP reg//////////////////////
    ////////////////////////////////////////////////////
    i2c_write(0xfe, 0x00);
    i2c_write(0x03, 0x04); //exposure[12:8]
    i2c_write(0x04, 0xe2); //exposure[7 :0]
    i2c_write(0x09, 0x00); //row_start[10:8]
    i2c_write(0x0a, 0x00); //row_start[7 :0]
    i2c_write(0x0b, 0x00); //col_start[10:8]
    i2c_write(0x0c, 0x00); //col_start[7 :1]
    i2c_write(0x0d, 0x04); //win_height[10:8]
    i2c_write(0x0e, 0xd0); //win_height[7 :0]
    i2c_write(0x0f, 0x06); //win_width[10:8]
    i2c_write(0x10, 0x48); //win_width[7 :1]
    i2c_write(0x12, 0x2e);
    i2c_write(0x17, 0x14); //[0]mirror, [1]updown
    i2c_write(0x18, 0x22);
    i2c_write(0x19, 0x0e);
    i2c_write(0x1a, 0x01);
    i2c_write(0x1b, 0x4b);
    i2c_write(0x1c, 0x07);
    i2c_write(0x1d, 0x10);
    i2c_write(0x1e, 0x88);
    i2c_write(0x1f, 0x78);
    i2c_write(0x20, 0x03);
    i2c_write(0x21, 0x40);
    i2c_write(0x22, 0xa0);
#ifndef NOAPP
    i2c_write(0x24, 0x2A); //0x16
#else
    i2c_write(0x24, 0x16);
#endif
    i2c_write(0x25, 0x01);
    i2c_write(0x26, 0x10);
    i2c_write(0x2d, 0x60);
    i2c_write(0x30, 0x01);
    i2c_write(0x31, 0x90);
    i2c_write(0x33, 0x06);
    i2c_write(0x34, 0x01);
    /////////////////////////////////////////////////
    //////////////////ISP reg////////////////////
    /////////////////////////////////////////////////
    i2c_write(0xfe, 0x00);
    i2c_write(0x80, 0x7f);
    i2c_write(0x81, 0x26);
    i2c_write(0x82, 0xfa);
    i2c_write(0x83, 0x00);
    i2c_write(0x84, 0x02);//output format
    i2c_write(0x86, 0x02);
    i2c_write(0x88, 0x03);
    i2c_write(0x89, 0x03);
    i2c_write(0x85, 0x08);
    i2c_write(0x8a, 0x00);
    i2c_write(0x8b, 0x00);
    i2c_write(0xb0, 0x55);
    i2c_write(0xc3, 0x00);
    i2c_write(0xc4, 0x80);
    i2c_write(0xc5, 0x90);
    i2c_write(0xc6, 0x3b);
    i2c_write(0xc7, 0x46);
    i2c_write(0xec, 0x06);
    i2c_write(0xed, 0x04);
    i2c_write(0xee, 0x60);
    i2c_write(0xef, 0x90);
    i2c_write(0xb6, AEC_ENABLE); //AEC ENABLE
    i2c_write(0x90, CROP_ENABLE);//CROP_ENABLE
    i2c_write(0x91, 0x00); //out_win_y1[10:8]
    i2c_write(0x92, 0x00); //out_win_y1[7:0]
    i2c_write(0x93, 0x00); //out_win_x1[10:8]
    i2c_write(0x94, 0x00); //out_win_x1[7:0]
    i2c_write(0x95, 0x04); //out_win_height[10:8]
    i2c_write(0x96, 0xb0); //out_win_height[7:0]
    i2c_write(0x97, 0x06); //out_win_width[10:8]
    i2c_write(0x98, 0x40); //out_win_width[7:0]
    /////////////////////////////////////////
    /////////// BLK ////////////////////////
    /////////////////////////////////////////
    i2c_write(0xfe, 0x00);
    i2c_write(0x40, 0x42);
    i2c_write(0x41, 0x00);
    i2c_write(0x43, 0x5b);
    i2c_write(0x5e, 0x00);
    i2c_write(0x5f, 0x00);
    i2c_write(0x60, 0x00);
    i2c_write(0x61, 0x00);
    i2c_write(0x62, 0x00);
    i2c_write(0x63, 0x00);
    i2c_write(0x64, 0x00);
    i2c_write(0x65, 0x00);
    i2c_write(0x66, 0x20);
    i2c_write(0x67, 0x20);
    i2c_write(0x68, 0x20);
    i2c_write(0x69, 0x20);
    i2c_write(0x76, 0x00);
    i2c_write(0x6a, 0x08);
    i2c_write(0x6b, 0x08);
    i2c_write(0x6c, 0x08);
    i2c_write(0x6d, 0x08);
    i2c_write(0x6e, 0x08);
    i2c_write(0x6f, 0x08);
    i2c_write(0x70, 0x08);
    i2c_write(0x71, 0x08);
    i2c_write(0x76, 0x00);
    i2c_write(0x72, 0xf0);
    i2c_write(0x7e, 0x3c);
    i2c_write(0x7f, 0x00);
    i2c_write(0xfe, 0x02);
    i2c_write(0x48, 0x15);
    i2c_write(0x49, 0x00);
    i2c_write(0x4b, 0x0b);
    i2c_write(0xfe, 0x00);
    ////////////////////////////////////////
    /////////// AEC ////////////////////////
    ////////////////////////////////////////
    //page[1]
    i2c_write(0xfe, 0x01);
    i2c_write(0x01, 0x04);
    i2c_write(0x02, 0xc0);
    i2c_write(0x03, 0x04);
    i2c_write(0x04, 0x90);
    i2c_write(0x05, 0x30);
    i2c_write(0x06, 0x90);
    i2c_write(0x07, 0x30);
    i2c_write(0x08, 0x80);
    i2c_write(0x09, 0x00);
    i2c_write(0x0a, 0x82);
    i2c_write(0x0b, 0x11);
    i2c_write(0x0c, 0x10);
    i2c_write(0x11, 0x10);
    i2c_write(0x13, 0x7b);
    i2c_write(0x17, 0x00);
    i2c_write(0x1c, 0x11);
    i2c_write(0x1e, 0x61);
    i2c_write(0x1f, 0x35);
    i2c_write(0x20, 0x40);
    i2c_write(0x22, 0x40);
    i2c_write(0x23, 0x20);
    //page[2]
    i2c_write(0xfe, 0x02);
    i2c_write(0x0f, 0x04);
    i2c_write(0xfe, 0x01);
    i2c_write(0x12, 0x35);
    i2c_write(0x15, 0xb0);
    i2c_write(0x10, 0x31);
    i2c_write(0x3e, 0x28);
    i2c_write(0x3f, 0xb0);
    i2c_write(0x40, 0x90);
    i2c_write(0x41, 0x0f);
    /////////////////////////////
    //////// INTPEE /////////////
    /////////////////////////////
    i2c_write(0xfe, 0x02);
    i2c_write(0x90, 0x6c);
    i2c_write(0x91, 0x03);
    i2c_write(0x92, 0xcb);
    i2c_write(0x94, 0x33);
    i2c_write(0x95, 0x84);
    i2c_write(0x97, 0x65);
    i2c_write(0xa2, 0x11);
    i2c_write(0xfe, 0x00);
    /////////////////////////////
    //////// DNDD///////////////
    /////////////////////////////
    i2c_write(0xfe, 0x02);
    i2c_write(0x80, 0xc1);
    i2c_write(0x81, 0x08);
    i2c_write(0x82, 0x05);
    i2c_write(0x83, 0x08);
    i2c_write(0x84, 0x0a);
    i2c_write(0x86, 0xf0);
    i2c_write(0x87, 0x50);
    i2c_write(0x88, 0x15);
    i2c_write(0x89, 0xb0);
    i2c_write(0x8a, 0x30);
    i2c_write(0x8b, 0x10);
    /////////////////////////////////////////
    /////////// ASDE ////////////////////////
    /////////////////////////////////////////
    i2c_write(0xfe, 0x01);
    i2c_write(0x21, 0x04);
    i2c_write(0xfe, 0x02);
    i2c_write(0xa3, 0x50);
    i2c_write(0xa4, 0x20);
    i2c_write(0xa5, 0x40);
    i2c_write(0xa6, 0x80);
    i2c_write(0xab, 0x40);
    i2c_write(0xae, 0x0c);
    i2c_write(0xb3, 0x46);
    i2c_write(0xb4, 0x64);
    i2c_write(0xb6, 0x38);
    i2c_write(0xb7, 0x01);
    i2c_write(0xb9, 0x2b);
    i2c_write(0x3c, 0x04);
    i2c_write(0x3d, 0x15);
    i2c_write(0x4b, 0x06);
    i2c_write(0x4c, 0x20);
    i2c_write(0xfe, 0x00);
    /////////////////////////////////////////
    /////////// GAMMA   ////////////////////////
    /////////////////////////////////////////
    i2c_write(0xfe, 0x02);
    i2c_write(0x10, 0x09);
    i2c_write(0x11, 0x0d);
    i2c_write(0x12, 0x13);
    i2c_write(0x13, 0x19);
    i2c_write(0x14, 0x27);
    i2c_write(0x15, 0x37);
    i2c_write(0x16, 0x45);
    i2c_write(0x17, 0x53);
    i2c_write(0x18, 0x69);
    i2c_write(0x19, 0x7d);
    i2c_write(0x1a, 0x8f);
    i2c_write(0x1b, 0x9d);
    i2c_write(0x1c, 0xa9);
    i2c_write(0x1d, 0xbd);
    i2c_write(0x1e, 0xcd);
    i2c_write(0x1f, 0xd9);
    i2c_write(0x20, 0xe3);
    i2c_write(0x21, 0xea);
    i2c_write(0x22, 0xef);
    i2c_write(0x23, 0xf5);
    i2c_write(0x24, 0xf9);
    i2c_write(0x25, 0xff);
    i2c_write(0xfe, 0x00);
    i2c_write(0xc6, 0x20);
    i2c_write(0xc7, 0x2b);

    i2c_write(0xfe, 0x02);
    i2c_write(0x26, 0x0f);
    i2c_write(0x27, 0x14);
    i2c_write(0x28, 0x19);
    i2c_write(0x29, 0x1e);
    i2c_write(0x2a, 0x27);
    i2c_write(0x2b, 0x33);
    i2c_write(0x2c, 0x3b);
    i2c_write(0x2d, 0x45);
    i2c_write(0x2e, 0x59);
    i2c_write(0x2f, 0x69);
    i2c_write(0x30, 0x7c);
    i2c_write(0x31, 0x89);
    i2c_write(0x32, 0x98);
    i2c_write(0x33, 0xae);
    i2c_write(0x34, 0xc0);
    i2c_write(0x35, 0xcf);
    i2c_write(0x36, 0xda);
    i2c_write(0x37, 0xe2);
    i2c_write(0x38, 0xe9);
    i2c_write(0x39, 0xf3);
    i2c_write(0x3a, 0xf9);
    i2c_write(0x3b, 0xff);
    ///////////////////////////////////////////////
    ///////////YCP ///////////////////////
    ///////////////////////////////////////////////
    i2c_write(0xfe, 0x02);
    i2c_write(0xd1, 0x32);
    i2c_write(0xd2, 0x32);
    i2c_write(0xd3, 0x40);
    i2c_write(0xd6, 0xf0);
    i2c_write(0xd7, 0x10);
    i2c_write(0xd8, 0xda);
    i2c_write(0xdd, 0x14);
    i2c_write(0xde, 0x86);
    i2c_write(0xed, 0x80);
    i2c_write(0xee, 0x00);
    i2c_write(0xef, 0x3f);
    i2c_write(0xd8, 0xd8);
    ///////////////////abs/////////////////
    i2c_write(0xfe, 0x01);
    i2c_write(0x9f, 0x40);
    /////////////////////////////////////////////
    //////////////////////// LSC ///////////////
    //////////////////////////////////////////
    i2c_write(0xfe, 0x01);
    i2c_write(0xc2, 0x14);
    i2c_write(0xc3, 0x0d);
    i2c_write(0xc4, 0x0c);
    i2c_write(0xc8, 0x15);
    i2c_write(0xc9, 0x0d);
    i2c_write(0xca, 0x0a);
    i2c_write(0xbc, 0x24);
    i2c_write(0xbd, 0x10);
    i2c_write(0xbe, 0x0b);
    i2c_write(0xb6, 0x25);
    i2c_write(0xb7, 0x16);
    i2c_write(0xb8, 0x15);
    i2c_write(0xc5, 0x00);
    i2c_write(0xc6, 0x00);
    i2c_write(0xc7, 0x00);
    i2c_write(0xcb, 0x00);
    i2c_write(0xcc, 0x00);
    i2c_write(0xcd, 0x00);
    i2c_write(0xbf, 0x07);
    i2c_write(0xc0, 0x00);
    i2c_write(0xc1, 0x00);
    i2c_write(0xb9, 0x00);
    i2c_write(0xba, 0x00);
    i2c_write(0xbb, 0x00);
    i2c_write(0xaa, 0x01);
    i2c_write(0xab, 0x01);
    i2c_write(0xac, 0x00);
    i2c_write(0xad, 0x05);
    i2c_write(0xae, 0x06);
    i2c_write(0xaf, 0x0e);
    i2c_write(0xb0, 0x0b);
    i2c_write(0xb1, 0x07);
    i2c_write(0xb2, 0x06);
    i2c_write(0xb3, 0x17);
    i2c_write(0xb4, 0x0e);
    i2c_write(0xb5, 0x0e);
    i2c_write(0xd0, 0x09);
    i2c_write(0xd1, 0x00);
    i2c_write(0xd2, 0x00);
    i2c_write(0xd6, 0x08);
    i2c_write(0xd7, 0x00);
    i2c_write(0xd8, 0x00);
    i2c_write(0xd9, 0x00);
    i2c_write(0xda, 0x00);
    i2c_write(0xdb, 0x00);
    i2c_write(0xd3, 0x0a);
    i2c_write(0xd4, 0x00);
    i2c_write(0xd5, 0x00);
    i2c_write(0xa4, 0x00);
    i2c_write(0xa5, 0x00);
    i2c_write(0xa6, 0x77);
    i2c_write(0xa7, 0x77);
    i2c_write(0xa8, 0x77);
    i2c_write(0xa9, 0x77);
    i2c_write(0xa1, 0x80);
    i2c_write(0xa2, 0x80);

    i2c_write(0xfe, 0x01);
    i2c_write(0xdf, 0x0d);
    i2c_write(0xdc, 0x25);
    i2c_write(0xdd, 0x30);
    i2c_write(0xe0, 0x77);
    i2c_write(0xe1, 0x80);
    i2c_write(0xe2, 0x77);
    i2c_write(0xe3, 0x90);
    i2c_write(0xe6, 0x90);
    i2c_write(0xe7, 0xa0);
    i2c_write(0xe8, 0x90);
    i2c_write(0xe9, 0xa0);
    i2c_write(0xfe, 0x00);
    ///////////////////////////////////////////////
    /////////// AWB////////////////////////
    ///////////////////////////////////////////////
    i2c_write(0xfe, 0x01);
    i2c_write(0x4f, 0x00);
    i2c_write(0x4f, 0x00);
    i2c_write(0x4b, 0x01);
    i2c_write(0x4f, 0x00);

    i2c_write(0x4c, 0x01); // D75
    i2c_write(0x4d, 0x71);
    i2c_write(0x4e, 0x01);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x91);
    i2c_write(0x4e, 0x01);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x70);
    i2c_write(0x4e, 0x01);

    i2c_write(0x4c, 0x01); // D65
    i2c_write(0x4d, 0x90);
    i2c_write(0x4e, 0x02);

    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xb0);
    i2c_write(0x4e, 0x02);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x8f);
    i2c_write(0x4e, 0x02);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x6f);
    i2c_write(0x4e, 0x02);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xaf);
    i2c_write(0x4e, 0x02);

    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xd0);
    i2c_write(0x4e, 0x02);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xf0);
    i2c_write(0x4e, 0x02);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xcf);
    i2c_write(0x4e, 0x02);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xef);
    i2c_write(0x4e, 0x02);

    i2c_write(0x4c, 0x01);//D50
    i2c_write(0x4d, 0x6e);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x8e);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xae);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xce);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x4d);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x6d);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x8d);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xad);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xcd);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x4c);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x6c);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x8c);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xac);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xcc);
    i2c_write(0x4e, 0x03);

    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xcb);
    i2c_write(0x4e, 0x03);

    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x4b);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x6b);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x8b);
    i2c_write(0x4e, 0x03);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xab);
    i2c_write(0x4e, 0x03);

    i2c_write(0x4c, 0x01);//CWF
    i2c_write(0x4d, 0x8a);
    i2c_write(0x4e, 0x04);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xaa);
    i2c_write(0x4e, 0x04);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xca);
    i2c_write(0x4e, 0x04);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xca);
    i2c_write(0x4e, 0x04);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xc9);
    i2c_write(0x4e, 0x04);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x8a);
    i2c_write(0x4e, 0x04);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0x89);
    i2c_write(0x4e, 0x04);
    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xa9);
    i2c_write(0x4e, 0x04);

    i2c_write(0x4c, 0x02);//tl84
    i2c_write(0x4d, 0x0b);
    i2c_write(0x4e, 0x05);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x0a);
    i2c_write(0x4e, 0x05);

    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xeb);
    i2c_write(0x4e, 0x05);

    i2c_write(0x4c, 0x01);
    i2c_write(0x4d, 0xea);
    i2c_write(0x4e, 0x05);

    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x09);
    i2c_write(0x4e, 0x05);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x29);
    i2c_write(0x4e, 0x05);

    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x2a);
    i2c_write(0x4e, 0x05);

    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x4a);
    i2c_write(0x4e, 0x05);

    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x8a);
    i2c_write(0x4e, 0x06);

    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x49);
    i2c_write(0x4e, 0x06);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x69);
    i2c_write(0x4e, 0x06);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x89);
    i2c_write(0x4e, 0x06);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0xa9);
    i2c_write(0x4e, 0x06);

    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x48);
    i2c_write(0x4e, 0x06);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x68);
    i2c_write(0x4e, 0x06);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0x69);
    i2c_write(0x4e, 0x06);

    i2c_write(0x4c, 0x02);//H
    i2c_write(0x4d, 0xca);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0xc9);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0xe9);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x03);
    i2c_write(0x4d, 0x09);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0xc8);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0xe8);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0xa7);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0xc7);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x02);
    i2c_write(0x4d, 0xe7);
    i2c_write(0x4e, 0x07);
    i2c_write(0x4c, 0x03);
    i2c_write(0x4d, 0x07);
    i2c_write(0x4e, 0x07);

    i2c_write(0x4f, 0x01);
    i2c_write(0x50, 0x80);
    i2c_write(0x51, 0xa8);
    i2c_write(0x52, 0x47);
    i2c_write(0x53, 0x38);
    i2c_write(0x54, 0xc7);
    i2c_write(0x56, 0x0e);
    i2c_write(0x58, 0x08);
    i2c_write(0x5b, 0x00);
    i2c_write(0x5c, 0x74);
    i2c_write(0x5d, 0x8b);
    i2c_write(0x61, 0xdb);
    i2c_write(0x62, 0xb8);
    i2c_write(0x63, 0x86);
    i2c_write(0x64, 0xc0);
    i2c_write(0x65, 0x04);

    i2c_write(0x67, 0xa8);
    i2c_write(0x68, 0xb0);
    i2c_write(0x69, 0x00);
    i2c_write(0x6a, 0xa8);
    i2c_write(0x6b, 0xb0);
    i2c_write(0x6c, 0xaf);
    i2c_write(0x6d, 0x8b);
    i2c_write(0x6e, 0x50);
    i2c_write(0x6f, 0x18);
    i2c_write(0x73, 0xf0);
    i2c_write(0x70, 0x0d);
    i2c_write(0x71, 0x60);
    i2c_write(0x72, 0x80);
    i2c_write(0x74, 0x01);
    i2c_write(0x75, 0x01);
    i2c_write(0x7f, 0x0c);
    i2c_write(0x76, 0x70);
    i2c_write(0x77, 0x58);
    i2c_write(0x78, 0xa0);
    i2c_write(0x79, 0x5e);
    i2c_write(0x7a, 0x54);
    i2c_write(0x7b, 0x58);
    i2c_write(0xfe, 0x00);
    //////////////////////////////////////////
    ///////////CC////////////////////////
    //////////////////////////////////////////
    i2c_write(0xfe, 0x02);
    i2c_write(0xc0, 0x01);
    i2c_write(0xc1, 0x44);
    i2c_write(0xc2, 0xfd);
    i2c_write(0xc3, 0x04);
    i2c_write(0xc4, 0xf0);
    i2c_write(0xc5, 0x48);
    i2c_write(0xc6, 0xfd);
    i2c_write(0xc7, 0x46);
    i2c_write(0xc8, 0xfd);
    i2c_write(0xc9, 0x02);
    i2c_write(0xca, 0xe0);
    i2c_write(0xcb, 0x45);
    i2c_write(0xcc, 0xec);
    i2c_write(0xcd, 0x48);
    i2c_write(0xce, 0xf0);
    i2c_write(0xcf, 0xf0);
    i2c_write(0xe3, 0x0c);
    i2c_write(0xe4, 0x4b);
    i2c_write(0xe5, 0xe0);
    //////////////////////////////////////////
    ///////////ABS ////////////////////
    //////////////////////////////////////////
    i2c_write(0xfe, 0x01);
    i2c_write(0x9f, 0x40);
    i2c_write(0xfe, 0x00);
    //////////////////////////////////////
    ///////////  OUTPUT   ////////////////
    //////////////////////////////////////
    i2c_write(0xfe, 0x00);
    i2c_write(0xf2, 0x00);

    //////////////frame rate 50Hz/////////
    i2c_write(0xfe, 0x00);
    i2c_write(0x05, 0x01);
    i2c_write(0x06, 0x56);
    i2c_write(0x07, 0x00);
    i2c_write(0x08, 0x32);

    i2c_write(0xfe, 0x01);
    i2c_write(0x25, 0x00);
    i2c_write(0x26, 0xfa);
    i2c_write(0x27, 0x04);
    i2c_write(0x28, 0xe2); //20fps
    i2c_write(0x29, 0x04);
    i2c_write(0x2a, 0xe2); //14fps
    i2c_write(0x2b, 0x04);
    i2c_write(0x2c, 0xe2); //12fps
    i2c_write(0x2d, 0x0e);
    i2c_write(0x2e, 0xe2); //8fps
    i2c_write(0xfe, 0x00);

    ///////////////dark sun////////////////////
    i2c_write(0xfe, 0x02);
    i2c_write(0x40, 0xbf);
    i2c_write(0x46, 0xcf);
    i2c_write(0xfe, 0x00);

    /////////////////////////////////////////////////////
    //////////////////////   MIPI   /////////////////////
    /////////////////////////////////////////////////////
    i2c_write(0xfe, 0x03); //page[3]
    i2c_write(0x02, 0x22);
    i2c_write(0x03, 0x10); // 0x12 20140821
    i2c_write(0x04, 0x10); // 0x01
    i2c_write(0x05, 0x00);
    i2c_write(0x06, 0x88);
#ifdef GC2145MIPI_2LANE
//    i2c_write(0x01, 0x87); //DPHY_analog_mode1: lane setting
//    i2c_write(0x10, 0x95); //CSI2_mod: [7]lane enable, [4]MIPI_enable [2]RAW8 [0]double_lan
#else
//    i2c_write(0x01, 0x83);
//    i2c_write(0x10, 0x94);
#endif
    i2c_write(0x11, 0x1e); //LDI_set
    i2c_write(0x13, 0x0c); //LWC_SET[15:8]
    i2c_write(0x12, 0x80); //LWC_SET[7:0]
    i2c_write(0x15, 0x10); //DPHY_mode
    i2c_write(0x17, 0xf0); //fifo_gate_mode
#ifndef NOAPP
    i2c_write(0x21, 0x08);
#else
    i2c_write(0x21, 0x10); //T_LPX_set
#endif
    i2c_write(0x22, 0x04); //T_CLK_HS_PREPARE_set
    i2c_write(0x23, 0x10);
    i2c_write(0x24, 0x10);
    i2c_write(0x25, 0x10);
    i2c_write(0x26, 0x05);
#ifndef NOAPP
    i2c_write(0x29, 0x03);
#else
    i2c_write(0x29, 0x01); //T_HS_PREPARE_set
#endif
    i2c_write(0x2a, 0x0a);
    i2c_write(0x2b, 0x06);

    i2c_write(0xfe, 0x00);

    i2c_write(0xfe, 0x00);
    i2c_write(0xfd, 0x00);
    i2c_write(0xfa, 0x11);

    i2c_write(0xfe, 0x00);
    i2c_write(0x90, CROP_ENABLE);//CROP_ENABLE
    i2c_write(0x91, 0x00);
    i2c_write(0x92, 0x00);
    i2c_write(0x93, 0x00);
    i2c_write(0x94, 0x00);
    i2c_write(0x95, 0x04);
    i2c_write(0x96, 0xb0);
    i2c_write(0x97, 0x06);
    i2c_write(0x98, 0x40);
    i2c_write(0x99, 0x11);
    i2c_write(0x9a, 0x06);

    i2c_write(0xfe, 0x00);
    i2c_write(0xec, 0x06);
    i2c_write(0xed, 0x04);
    i2c_write(0xee, 0x60);
    i2c_write(0xef, 0x90);
    i2c_write(0xfe, 0x01);
    i2c_write(0x74, 0x01);

    i2c_write(0xfe, 0x01);
    i2c_write(0x01, 0x04);
    i2c_write(0x02, 0xc0);
    i2c_write(0x03, 0x04);
    i2c_write(0x04, 0x90);
    i2c_write(0x05, 0x30);
    i2c_write(0x06, 0x90);
    i2c_write(0x07, 0x30);
    i2c_write(0x08, 0x80);
    i2c_write(0x0a, 0x82);

    i2c_write(0xfe, 0x01);
    i2c_write(0x21, 0x15);
    i2c_write(0xfe, 0x00);
    i2c_write(0x20, 0x15);

    i2c_write(0xfe, 0x03);
    i2c_write(0x12, 0x80);
    i2c_write(0x13, 0x0c);
    i2c_write(0x04, 0x01);
    i2c_write(0x05, 0x00);
    i2c_write(0xfe, 0x00);
}

static void sensor_svga_init(void)
{
    i2c_write(0xfe, 0x00);
    i2c_write(0xfd, CROP_ENABLE);//scalar mode, [0]: scalar mode, [1]:colume scalar mode
    i2c_write(0xfa, 0x00);

    //// crop window
    i2c_write(0xfe, 0x00);
    i2c_write(0x90, CROP_ENABLE);//CROP_ENABLE
    i2c_write(0x91, 0x00);//out_win_y1[10:8]
    i2c_write(0x92, 0x00);//out_win_y1[7:0]
    i2c_write(0x93, 0x00);//out_win_x1[10:8]
    i2c_write(0x94, 0x00);//out_win_x1[7:0]
    i2c_write(0x95, 0x02);//out_win_height[10:8]
    i2c_write(0x96, 0x58);//out_win_height[7:0]
    i2c_write(0x97, 0x03);//out_win_width[10:8]
    i2c_write(0x98, 0x20);//out_win_width[7:0]
    i2c_write(0x99, 0x11);
    i2c_write(0x9a, 0x06);
    //// AWB
    i2c_write(0xfe, 0x00);
    i2c_write(0xec, 0x02);
    i2c_write(0xed, 0x02);
    i2c_write(0xee, 0x30);
    i2c_write(0xef, 0x48);
    i2c_write(0xfe, 0x02);
    i2c_write(0x9d, 0x08);
    i2c_write(0xfe, 0x01);
    i2c_write(0x74, 0x00);
    //// AEC
    i2c_write(0xfe, 0x01);
    i2c_write(0x01, 0x04);
    i2c_write(0x02, 0x60);
    i2c_write(0x03, 0x02);
    i2c_write(0x04, 0x48);
    i2c_write(0x05, 0x18);
    i2c_write(0x06, 0x50);
    i2c_write(0x07, 0x10);
    i2c_write(0x08, 0x38);
    i2c_write(0x0a, 0x80);
    i2c_write(0x21, 0x04);
    i2c_write(0xfe, 0x00);
    i2c_write(0x20, 0x03);

    //// mipi
    i2c_write(0xfe, 0x03);
    i2c_write(0x12, 0x40);
    i2c_write(0x13, 0x06);
#ifdef GC2145MIPI_2LANE
    i2c_write(0x04, 0x90);
    i2c_write(0x05, 0x01);
#else
    i2c_write(0x04, 0x01);
    i2c_write(0x05, 0x00);
#endif
    i2c_write(0xfe, 0x00);
}

POSSIBLY_UNUSED static void csi_sensor_phy_enable(void)
{
    sensor_reg_page(3);

#ifdef GC2145MIPI_2LANE
    i2c_write(0x01, 0x87); //DPHY_analog_mode1: lane setting
    i2c_write(0x10, 0x95); //CSI2_mod: [7]lane enable, [4]MIPI_enable [2]RAW8 [0]double_lan
#else
    i2c_write(0x01, 0x83);
    i2c_write(0x10, 0x94);
#endif
    sensor_reg_page(0);
}

POSSIBLY_UNUSED static void csi_sensor_phy_disable(void)
{
    sensor_reg_page(3);
    i2c_write(0x01, 0x00); //DPHY_analog_mode1: lane setting
    i2c_write(0x10, 0x00); //CSI2_mod: [7]lane enable, [4]MIPI_enable [2]RAW8 [0]double_lan
    sensor_reg_page(0);
}

POSSIBLY_UNUSED static void camera_test_pettern(void)
{
    i2c_write(0xfe, 0x00);
    i2c_write(0xb6, 0x00);
    i2c_write(0x80, 0x08);
    i2c_write(0x81, 0x00);
    i2c_write(0x82, 0x00);
    i2c_write(0x03, 0x06);
    i2c_write(0x04, 0x40);
    i2c_write(0x40, 0x00);
    i2c_write(0x5e, 0x00);
    i2c_write(0x5f, 0x00);
    i2c_write(0x60, 0x00);
    i2c_write(0x61, 0x00);
    i2c_write(0x62, 0x00);
    i2c_write(0x63, 0x00);
    i2c_write(0x64, 0x00);
    i2c_write(0x65, 0x00);
    i2c_write(0x66, 0x00);
    i2c_write(0x67, 0x00);
    i2c_write(0x68, 0x00);
    i2c_write(0x69, 0x00);
    i2c_write(0xb0, 0x60);
    i2c_write(0xb1, 0x20);
    i2c_write(0xb2, 0x40);
    i2c_write(0xb3, 0x40);
    i2c_write(0xb4, 0x40);
    i2c_write(0xb5, 0x40);
    i2c_write(0xfe, 0x02);
    i2c_write(0x3c, 0x00);
    i2c_write(0x4b, 0x00);
    i2c_write(0x49, 0x00);
    i2c_write(0xdd, 0x00);
    i2c_write(0xfe, 0x00);
    i2c_write(0x8c, 0x01);
}

static int sensor_get_chip_id(uint8_t cid[])
{
    int err = 0;
    uint8_t chip_id[2] = {0};

    i2c_read(0xF0, &chip_id[0]);
    i2c_read(0xF1, &chip_id[1]);

    if (cid) {
        cid[0] = chip_id[0];
        cid[1] = chip_id[1];
    }
    if ((chip_id[0] == 0x21) && (chip_id[1] == 0x45)) {
        TRACE(1, "[%s]: GOOD CAMERA CHIP ID: [%x-%x]", __func__, chip_id[0],chip_id[1]);
    } else {
        TRACE(1, "[%s]: BAD CAMERA CHIP ID: [%x-%x]", __func__, chip_id[0],chip_id[1]);
        err = 1;
    }
    return err;
}

static void sensor_init(void)
{
    camera_device_hw_pin_init();
}

static void sensor_power_on(bool on)
{
    TRACE(0, "[%s]: on=%d", __func__, on);

    if (sens_pwr_status == on) {
        TRACE(1, "[%s]: status done: %d" ,__func__, on);
        return;
    }
    if (on) {
        camera_device_hw_power_on(sens_dev_id, sens_dev_type);
        camera_device_hw_i2c_open(sens_i2c_user, sens_i2c_id, sens_i2c_speed);
    } else {
        camera_device_hw_power_off(sens_dev_id, sens_dev_type);
        camera_device_hw_i2c_close(sens_i2c_user, sens_i2c_id);
    }
    sens_pwr_status = on;
}

static int sensor_open(void)
{
    uint32_t time;

    time = hal_sys_timer_get();

    TRACE(0, "[%s]", __func__);

    sensor_get_chip_id(NULL);
    sensor_clock_init();
    time = hal_sys_timer_get() - time;
    TRACE(0, "[%s]: okay, cost %d ms", __func__, TICKS_TO_MS(time));
    return 0;
}

static int sensor_get_op_flag(void)
{
    return sens_dev_cfg.op_flag;
}

static int sensor_get_config(struct VP_DEVICE_CONFIG_T *cfg)
{
    if (cfg) {
        *cfg = sens_dev_cfg;
    }
    return 0;
}

static int sensor_set_config(struct VP_DEVICE_CONFIG_T *cfg)
{
    if (cfg) {
        sens_dev_cfg = *cfg;
    }
    return 0;
}

static int sensor_test_mode(int mode, bool enable)
{
    if (enable) {
        if (mode == 0) {
            camera_test_pettern();
        } else {
            sensor_dbg_mode_set_fix_value(mode);
        }
    } else {
        //TODO: back to normal mode
    }
    return 0;
}

static int sensor_reset(void)
{
    i2c_write(0xFE, 0xF0);
    i2c_write(0xFE, 0xF0);
    i2c_write(0xFE, 0xF0);
    osDelay(5);

    return 0;
}

static int sensor_start(void)
{
    TRACE(0, "[%s]", __func__);

    sensor_common_init();
    sensor_svga_init();

#ifdef CAMERA_TEST_MODE_INIT_ON
    camera_test_pettern();
    sensor_dbg_mode_set_fix_value(5);
#endif

    csi_sensor_phy_enable();
    return 0;
}

static int sensor_stop(void)
{
    TRACE(0, "[%s]", __func__);

    csi_sensor_phy_disable();
    return 0;
}

static int sensor_close(void)
{
    TRACE(0, "[%s]", __func__);

    sensor_reset();
    return 0;
}

static int sensor_get_bus(struct VP_DEVICE_BUS_T *bus)
{
    if (bus) {
        bus->user = sens_i2c_user;
        bus->id = sens_i2c_id;
        bus->addr = sens_i2c_addr;
        bus->speed = sens_i2c_speed;
#ifdef CSI_SENS_I2C_DEBUG
        TRACE(1, "[%s]: user=%d,id=%d,addr=0x%x,speed=%d",
            __func__, bus->user, bus->id, bus->addr, bus->speed);
#endif
    }

    return 0;
}

static int sensor_set_bus(struct VP_DEVICE_BUS_T *bus)
{
    if (bus) {
        sens_i2c_user = (enum CAMERA_I2C_USER_T)(bus->user);
        sens_i2c_id = (enum CAMERA_I2C_ID_T)(bus->id);
        sens_i2c_addr = bus->addr;
        sens_i2c_speed = bus->speed;
#ifdef CSI_SENS_I2C_DEBUG
        TRACE(1, "[%s]: user=%d,id=%d,addr=0x%x,speed=%d",
            __func__, bus->user, bus->id, bus->addr, bus->speed);
#endif
    }
    return 0;
}

#ifdef CAMERA_DUMMY_DEVICE
static void sensor_dummy_init(void) { }
static void sensor_dummy_power_on(bool on) { }
static int sensor_dummy_get_chip_id(uint8_t cid[]) {return 0;}
static int sensor_dummy_open(void){return 0;}
static int sensor_dummy_start(void){return 0;}
static int sensor_dummy_stop(void){return 0;}
static int sensor_dummy_close(void){return 0;}
static int sensor_dummy_reset(void){return 0;}
static int sensor_dummy_bus_write(uint8_t reg_addr, uint8_t val) {return 0;}
static int sensor_dummy_bus_read(uint8_t reg_addr, uint8_t *val) {return 0;}
static int sensor_dummy_test_mode(int mode, bool enable) {return 0;}
static int sensor_dummy_get_config(struct VP_DEVICE_CONFIG_T *cfg) {return 0;}
static int sensor_dummy_set_config(struct VP_DEVICE_CONFIG_T *cfg) {return 0;}
static int sensor_dummy_get_op_flag(void) {return 0;}
static int sensor_dummy_get_bus(struct VP_DEVICE_BUS_T *bus) {return 0;}
static int sensor_dummy_set_bus(struct VP_DEVICE_BUS_T *bus) {return 0;}

static const struct VP_DEVICE_OPS_T gc2145_dum_ops = {
    .name = "GC2145_DUMMY",
    .init = sensor_dummy_init,
    .power_on = sensor_dummy_power_on,
    .get_chip_id = sensor_dummy_get_chip_id,
    .open = sensor_dummy_open,
    .start = sensor_dummy_start,
    .stop = sensor_dummy_stop,
    .close = sensor_dummy_close,
    .reset = sensor_dummy_reset,
    .bus_write = sensor_dummy_bus_write,
    .bus_read = sensor_dummy_bus_read,
    .test_mode = sensor_dummy_test_mode,
    .get_config = sensor_dummy_get_config,
    .set_config = sensor_dummy_set_config,
    .get_op_flag = sensor_dummy_get_op_flag,
    .get_bus = sensor_dummy_get_bus,
    .set_bus = sensor_dummy_set_bus,
};
#endif

static const struct VP_DEVICE_OPS_T gc2145_ops = {
    .name = "GC2145",
    .init = sensor_init,
    .power_on = sensor_power_on,
    .get_chip_id = sensor_get_chip_id,
    .open = sensor_open,
    .start = sensor_start,
    .stop = sensor_stop,
    .close = sensor_close,
    .reset = sensor_reset,
    .bus_write = sensor_bus_write,
    .bus_read = sensor_bus_read,
    .test_mode = sensor_test_mode,
    .get_config = sensor_get_config,
    .set_config = sensor_set_config,
    .get_op_flag = sensor_get_op_flag,
    .get_bus = sensor_get_bus,
    .set_bus = sensor_set_bus,
};

struct VP_DEVICE_OPS_T *gc2145_sensor_ops_get(void)
{
    const struct VP_DEVICE_OPS_T *ops;

#ifdef CAMERA_DUMMY_DEVICE
    ops = &gc2145_dum_ops;
#else
    ops = &gc2145_ops;
#endif
    return (struct VP_DEVICE_OPS_T *)ops;
}

void gc2145_sensor_page_select(int page)
{
    uint8_t v = page & 0x7;
    i2c_write(0xFE, v);
}

#endif /* CSI_SENS_GC2145 */
