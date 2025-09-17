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
#ifdef CSI_SENS_SC132GS
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
#include "camera_sens_sc132gs.h"
#include "camera_drv.h"

#ifndef SC132GS_IMAGE_WIDTH
#define SC132GS_IMAGE_WIDTH 480
#endif
#ifndef SC132GS_IMAGE_HEIGHT
#define SC132GS_IMAGE_HEIGHT 640
#endif

#define CAM_IMAGE_MAX_WIDTH  1080
#define CAM_IMAGE_MAX_HEIGHT 1280


#ifdef CSI_SLT_TEST
#define SC132GS_OP_FLAG ( 0 \
    | CAMERA_OP_FLAG_RAW10_TO_YUV \
    | CAMERA_OP_FLAG_CALIB_PHY \
    | CAMERA_OP_FLAG_NULL)
#else
#define SC132GS_OP_FLAG ( 0 \
    | CAMERA_OP_FLAG_SOFT_ISP \
    | CAMERA_OP_FLAG_RAW10_TO_YUV \
    | CAMERA_OP_FLAG_CALIB_PHY \
    | CAMERA_OP_FLAG_NULL)
#endif

//#define CSI_SENS_I2C_DEBUG

//#define CSI_SENS_TEST_PATT

struct IMG_SENS_REG_T {
    uint16_t addr;
    uint8_t val;
};

#define I2C_USER_SC132GS     (CAMERA_I2C_USER_1)
#define I2C_ID_SC132GS       (CAMERA_I2C_ID_0)
#define I2C_SPEED_SC132GS    (CAMERA_I2C_SPEED_200KHZ)
#define I2C_ADDR_SC132GS     (0x30) //31,32,33 depend on SID0, SID1

static bool sens_pwr_status = false;

static enum VP_DEVICE_T sens_dev_type = VP_DEVICE_SC132GS;
static enum VP_DEVICE_ID_T sens_dev_id = VP_DEVICE_ID_0;
static enum CAMERA_I2C_USER_T sens_i2c_user = I2C_USER_SC132GS;
static enum CAMERA_I2C_ID_T   sens_i2c_id = I2C_ID_SC132GS;
static uint8_t sens_i2c_addr = I2C_ADDR_SC132GS;
static uint32_t sens_i2c_speed = I2C_SPEED_SC132GS;

static struct VP_DEVICE_CONFIG_T sens_dev_cfg = {
    .width = SC132GS_IMAGE_WIDTH,
    .height = SC132GS_IMAGE_HEIGHT,
    .fmt = VP_PIXEL_FMT_YUV422,
    .channel_num = VP_CHANNEL_NUM_2,
    .op_flag = SC132GS_OP_FLAG,
};

////////////////////////////////////////////////////////////////////////////
//60fps, raw10, MIPI clk=300MHz
static struct IMG_SENS_REG_T sens_reg_cfg_mipi_300mhz[] = {
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x3018,0x32},
    {0x3019,0x0c},
    {0x301a,0xb4},
    {0x301f,0x42},
    {0x3031,0x0a}, //[3:0]: MIPI data format: 8=raw8, a=raw10, c=raw12
    {0x3037,0x00}, //[6:5]: PHY data mode:  0=8bit, 1=10bit 2=12bit
    {0x3032,0x60},
    {0x3038,0x44},
    {0x3207,0x17},
    {0x3208,0x04},// width=1080
    {0x3209,0x38},//
    {0x320a,0x05},// heigth=1280
    {0x320b,0x00},//
    {0x320c,0x02},
    {0x320d,0xee},
    {0x320e,0x05},
    {0x320f,0x78},
    {0x3250,0xcc},
    {0x3251,0x02},
    {0x3252,0x05},
    {0x3253,0x73},
    {0x3254,0x05},
    {0x3255,0x3b},
    {0x3306,0x78},
    {0x330a,0x00},
    {0x330b,0xc8},
    {0x330f,0x24},
    {0x3314,0x80},
    {0x3315,0x40},
    {0x3317,0xf0},
    {0x331f,0x12},
    {0x3364,0x00},
    {0x3385,0x41},
    {0x3387,0x41},
    {0x3389,0x09},
    {0x33ab,0x00},
    {0x33ac,0x00},
    {0x33b1,0x03},
    {0x33b2,0x12},
    {0x33f8,0x02},
    {0x33fa,0x01},
    {0x3409,0x08},
    {0x34f0,0xc0},
    {0x34f1,0x20},
    {0x34f2,0x03},
    {0x3622,0xf5},
    {0x3630,0x5c},
    {0x3631,0x80},
    {0x3632,0xc8},
    {0x3633,0x32},
    {0x3638,0x2a},
    {0x3639,0x07},
    {0x363b,0x48},
    {0x363c,0x83},
    {0x363d,0x10},
    //{0x3651,0x7F},
    {0x3654,0x00}, //mipi clock delay
    {0x36ea,0x36},
    {0x36eb,0x04},
    {0x36ec,0x03},
    {0x36ed,0x24},
    {0x36fa,0x2b},
    {0x36fb,0x0b},
    {0x36fc,0x01},
    {0x36fd,0x34},
    {0x3900,0x11},
    {0x3901,0x05},
    {0x3902,0xc5},
    {0x3904,0x04},
    {0x3908,0x91},
    {0x391e,0x00},
    {0x3e01,0x57},
    {0x3e02,0x00},
    {0x3e09,0x20},
    {0x3e0e,0xd2},
    {0x3e14,0xb0},
    {0x3e1e,0x7c},
    {0x3e26,0x20},
    {0x4418,0x38},
    {0x4503,0x10},
#ifdef SC132GS_CONTINUE_CLOCK_MODE
    {0x4800,0x04},
#else
    {0x4800,0x24},
#endif
    {0x4837,0x1a},
    {0x5000,0x0e},
    {0x540c,0x51},
    {0x550f,0x38},
    {0x5780,0x67},
    {0x5784,0x10},
    {0x5785,0x06},
    {0x5787,0x02},
    {0x5788,0x00},
    {0x5789,0x00},
    {0x578a,0x02},
    {0x578b,0x00},
    {0x578c,0x00},
    {0x5790,0x00},
    {0x5791,0x00},
    {0x5792,0x00},
    {0x5793,0x00},
    {0x5794,0x00},
    {0x5795,0x00},
    {0x5799,0x04},
    {0x36e9,0x54},
    {0x36f9,0x50},
    {0x0100,0x01},

//    {[gain<2]
    {0x33fa,0x01},
    {0x3317,0xf0},

//    {[gain>=2]
    {0x33fa,0x02},
    {0x3317,0x14},
};

/*640*480, 120fps */
static struct IMG_SENS_REG_T sens_reg_cfg_mipi_150mhz[] = {
    {0x0103,0x01},//
    {0x0100,0x00},//
    {0x36e9,0x80},//
    {0x36f9,0x80},//
    {0x3018,0x32},
    {0x3019,0x0c},
    {0x301a,0xb4},
    {0x301f,0x17},//
    {0x3032,0x60},//
    {0x3038,0x44},//
    {0x3200,0x00},//
    {0x3201,0x3c},//
    {0x3202,0x00},//
    {0x3203,0x00},//
    {0x3204,0x04},//
    {0x3205,0x1b},//
    {0x3206,0x05},//
    {0x3207,0x1f},//
    {0x3208,0x01},// width=480
    {0x3209,0xe0},//
    {0x320a,0x02},// heigth=640
    {0x320b,0x80},//
    {0x320c,0x02},
    {0x320d,0xee},
    {0x320e,0x05},//
    {0x320f,0x78},//
    {0x3210,0x00},//
    {0x3211,0x08},//
    {0x3212,0x00},//
    {0x3213,0x08},//
    {0x3215,0x22},//
    {0x3220,0x87},//
    {0x3230,0x04},//
    {0x3231,0x2a},//
    {0x3250,0xcc},
    {0x3251,0x02},
    {0x3252,0x02},//
    {0x3253,0xb7},//
    {0x3254,0x05},
    {0x3255,0x3b},
    {0x3306,0x78},
    {0x330a,0x00},
    {0x330b,0xc8},
    {0x330f,0x24},
    {0x3314,0xa0},//
    {0x3315,0x40},
    {0x3317,0xf0},
    {0x331f,0x12},
    {0x334f,0xbe},//
    {0x3364,0x00},
    {0x3385,0x41},
    {0x3387,0x41},
    {0x3389,0x09},
    {0x33ab,0x00},
    {0x33ac,0x00},
    {0x33b1,0x03},
    {0x33b2,0x12},
    {0x33f8,0x02},
    {0x33fa,0x01},
    {0x3409,0x08},
    {0x34f0,0xc0},
    {0x34f1,0x20},
    {0x34f2,0x03},
    {0x3622,0xf5},
    {0x3630,0x5c},
    {0x3631,0x80},
    {0x3632,0xc8},//
    {0x3633,0x32},
    {0x3638,0x2a},
    {0x3639,0x07},
    {0x363b,0x48},
    {0x363c,0x83},
    {0x363d,0x10},
    {0x36ea,0x3b},//
    {0x36eb,0x04},
    {0x36ec,0x13},//
    {0x36ed,0x24},
    {0x36fa,0x39},//
    {0x36fb,0x15},//
    {0x36fc,0x01},//
    {0x36fd,0x34},//
    {0x3900,0x11},//
    {0x3901,0x05},
    {0x3902,0xc5},//
    {0x3904,0x04},
    {0x3908,0x91},
    {0x391e,0x00},
    {0x3e00,0x00},
    {0x3e01,0x2b},
    {0x3e02,0x40},
    {0x3e03,0x0b},
    {0x3e08,0x03},
    {0x3e09,0x20},
    {0x3e0e,0xd2},
    {0x3e14,0xb0},
    {0x3e1e,0x7c},
    {0x3e26,0x20},
    {0x4418,0x38},
    {0x4503,0x10},
#ifdef SC132GS_CONTINUE_CLOCK_MODE
    {0x4800,0x04},
#else
    {0x4800,0x24},
#endif
    {0x4837,0x34},//
    {0x5000,0x4e},//
    {0x540c,0x51},//
    {0x550f,0x38},//
    {0x5780,0x67},
    {0x5784,0x10},
    {0x5785,0x06},
    {0x5787,0x02},
    {0x5788,0x00},
    {0x5789,0x00},
    {0x578a,0x02},
    {0x578b,0x00},
    {0x578c,0x00},
    {0x5790,0x00},
    {0x5791,0x00},
    {0x5792,0x00},
    {0x5793,0x00},
    {0x5794,0x00},
    {0x5795,0x00},
    {0x5799,0x04},
    {0x5900,0x05},
    {0x5901,0x14},//
    {0x36e9,0x24},//
    {0x36f9,0x53},//
    {0x0100,0x01},//
   // [gain<2]
    {0x33fa,0x01},
    {0x3317,0xf0},

    //[gain>=2]
    {0x33fa,0x02},
    {0x3317,0x14},
};

static struct IMG_SENS_REG_T sens_reg_dump_list[] = {
    {0x4b01, 0x00}, //[3]  : 1=LVDS, 0=MIPI
    {0x3018, 0x00}, //[7:5]: mipi lane number - 1
    {0x3031, 0x00}, //[3:0]: mipi bit mode: 8=raw8,10=raw10, c=raw12
    {0x3037, 0x00}, //[6:5]: phy bit mode, 0=8bit, 1=10bit, 2=12bit
    {0x303f, 0x00}, //[7]  : 0=pclk sel
    {0x301f, 0x00}, //
    {0x3651, 0x00}, //[3:0]: mipi driver mode
    {0x3652, 0x00}, //mipi lane 0/1 delay
    {0x3653, 0x00}, //mipi lane 2/3 delay
    {0x3654, 0x00}, //mipi clock delay

    //trigger mode config
    {0x3222, 0x00}, //[1]: trigger mode, 1=enable, 0=disable
    {0x3260, 0x00}, //active rows[15:8]
    {0x3261, 0x00}, //active rows[7 :0]
    {0x3228, 0x00}, //blank  rows[15:8]
    {0x3229, 0x00}, //blank  rows[7:0]

    {0x3220, 0x00}, //[6]: HDR mode: 1=enable, 0=disable
    {0x3e00, 0x00}, //total exp time, unit=1/16 line
    {0x3e01, 0x00}, //total exp time, unit=1/16 line
    {0x3e02, 0x00}, //total exp time, unit=1/16 line
    {0x3e08, 0x00}, //ANA gain regsiter
    {0x3e09, 0x00}, //ANA fine gain register
    {0x3e31, 0x00}, //HDR exp time, unit=1/16 line
    {0x3e32, 0x00}, //HDR exp time, unit=1/16 line

    {0x3222, 0x00}, //HDR point
    {0x5001, 0x00}, //HDRC
    {0x3221, 0x00}, //[2:1]: mirror ctrl, [6:5]: flip ctrl, 0=off, 3=on

    //frame length
    {0x320e, 0x00},
    {0x320f, 0x00},

    //window
    {0x3208, 0x00}, //width[15:8]
    {0x3209, 0x00}, //width[7:0]
    {0x320a, 0x00}, //height[15:8]
    {0x320b, 0x00}, //height[7:0]

    {0x3210, 0x00}, //start height [15:8]
    {0x3211, 0x00}, //start height [7:0]
    {0x3212, 0x00}, //start width  [15:8]
    {0x3213, 0x00}, //start width  [7:0]

    //test mode
    {0x4501, 0x00}, //[3]
    {0x3902, 0x00},
    {0x391d, 0x00},
    {0x3e06, 0x00},

    //clock mode
    {0x4800, 0x00},
};

static int sens_reg_list_set_image_size(struct IMG_SENS_REG_T *list, uint32_t num,
    uint32_t width, uint32_t height)
{
    uint32_t i, flag = 0;
    uint8_t w[2], h[2];

    TRACE(1, "[%s]: image: width=%d, height=%d", __func__, width, height);

    if (width > CAM_IMAGE_MAX_WIDTH) {
        TRACE(1, "[%s]: Bad image width=%d", __func__, width);
        return -1;
    }
    if (height > CAM_IMAGE_MAX_HEIGHT) {
        TRACE(1, "[%s]: Bad image height=%d", __func__, height);
        return -2;
    }
    w[0] = width & 0xFF;
    w[1] = (width >> 8) & 0xFF;
    h[0] = height & 0xFF;
    h[1] = (height >> 8) & 0xFF;
    flag = 0xF;

    for (i = 0; i < num; i++) {
        if (flag == 0) {
            break;
        }
        if (list[i].addr == 0x3208) {
            list[i].val = w[1];
            flag &= ~0x01;
        } else if (list[i].addr == 0x3209) {
            list[i].val = w[0];
            flag &= ~0x02;
        } else if (list[i].addr == 0x320A) {
            list[i].val = h[1];
            flag &= ~0x04;
        } else if (list[i].addr == 0x320B) {
            list[i].val = h[0];
            flag &= ~0x08;
        }
    }
    if (flag) {
        TRACE(1, "[%s]: WARNING: set image size error, flag=%x", __func__, flag);
        return -3;
    }
    return 0;
}

static int i2c_read(uint16_t reg_addr, uint8_t *val)
{
    int ret;
    uint8_t buf[4] = {0};
    uint32_t len = 0;

    buf[len++] = reg_addr >> 8; //[15:8]
    buf[len++] = reg_addr >> 0; //[7:0]
    ret = camera_device_hw_i2c_recv(sens_i2c_user, sens_i2c_id, sens_i2c_addr,
        buf, len, buf + len, 1);
    if (ret) {
        TRACE(1, "[%s]: WARNING: read error, ret=%x", __func__, ret);
        return -1;
    }
    if(val) {
        *val = buf[len];
    }

#ifdef CSI_SENS_I2C_DEBUG
    TRACE(0, "I2C_RD:[%02x]=%02x", reg_addr, buf[1]);
#endif

    return 0;
}

static int i2c_write(uint16_t reg_addr, uint8_t val)
{
    uint8_t buf[4];
    uint32_t len = 0;
    int ret;

#ifdef CSI_SENS_I2C_DEBUG
    TRACE(0, "I2C_WR:[%02x]=%02x", reg_addr, val);
#endif

    buf[len++] = reg_addr >> 8;
    buf[len++] = reg_addr >> 0;
    buf[len++] = val;
    ret = camera_device_hw_i2c_send(sens_i2c_user, sens_i2c_id, sens_i2c_addr, buf, len);
    if (ret) {
        TRACE(1, "[%s]: WARNING: write error, ret=%x", __func__, ret);
        return -1;
    }
    return 0;
}

static int sensor_bus_read(uint32_t reg_addr, uint32_t *reg_val)
{
    return i2c_read((uint16_t)reg_addr, (uint8_t *)reg_val);
}

static int sensor_bus_write(uint32_t reg_addr, uint32_t reg_val)
{
    return i2c_write((uint16_t)reg_addr, (uint8_t)reg_val);
}

static void sensor_clock_init(void)
{
    //PLL bypass
    i2c_write(0x36e9,0x80);
    i2c_write(0x36f9,0x80);
}

uint32_t sc132gs_sensor_calc_mipi_clock(uint32_t width, uint32_t height, uint32_t bits, uint32_t fps)
{
    uint32_t mipi_clk;

    if ((width == CAM_IMAGE_MAX_WIDTH) && (height == CAM_IMAGE_MAX_HEIGHT)) {
        mipi_clk = 300000000;
    } else {
        // 4: DDR mode, 2 DATA LANE
        mipi_clk = width * height * bits * fps / 4 * 3 / 2;
    }
    return mipi_clk;
}

static void sensor_common_init(void)
{
    struct IMG_SENS_REG_T *regs = NULL;
    uint32_t i, num = 0;
    uint32_t mipi_clk;
    int ret;

    uint32_t image_width, image_height;

    image_width = sens_dev_cfg.width;
    image_height = sens_dev_cfg.height;

    TRACE(1, "[%s]: image: width=%d, height=%d", __func__, image_width, image_height);

    mipi_clk = sc132gs_sensor_calc_mipi_clock(image_width, image_height, 10, 60);
    if (mipi_clk >= 150000000) {
        regs = sens_reg_cfg_mipi_300mhz;
        num = ARRAY_SIZE(sens_reg_cfg_mipi_300mhz);
        TRACE(1, "MIPI CLOCK: %d, select 300MHz list", mipi_clk);
    } else {
        regs = sens_reg_cfg_mipi_150mhz;
        num = ARRAY_SIZE(sens_reg_cfg_mipi_150mhz);
        TRACE(1, "MIPI CLOCK: %d, select 150MHz list", mipi_clk);
    }
    ret = sens_reg_list_set_image_size(regs, num, image_width, image_height);
    if (ret) {
        TRACE(1, "[%s]: WARNING: set image size failed, ret=%d", __func__, ret);
        return;
    }
    for (i = 0; i < num; i++) {
        i2c_write(regs[i].addr, regs[i].val);
    }
}

static uint32_t sensor_reg_get_value(uint16_t addr)
{
    uint32_t i;
    struct IMG_SENS_REG_T *regs = sens_reg_dump_list;

    for (i = 0; i < ARRAY_SIZE(sens_reg_dump_list); i++) {
        if (regs[i].addr == addr) {
            return regs[i].val;
        }
    }
    return 0;
}

static void sensor_regs_dump(void)
{
    struct IMG_SENS_REG_T *regs = sens_reg_dump_list;
    uint32_t i;

    TRACE(1, "DUMP_SENS_REGS:");
    for (i = 0; i < ARRAY_SIZE(sens_reg_dump_list); i++) {
        i2c_read(regs[i].addr, &regs[i].val);
        TRACE(1, "SENS REG[%04X]=%02X", regs[i].addr, regs[i].val);
    }
    TRACE(1, "DUMP_END.");

    uint32_t w, h, t[2];

    t[0] = sensor_reg_get_value(0x3208);
    t[1] = sensor_reg_get_value(0x3209);
    w = (t[0]<<8)|t[1];
    t[0] = sensor_reg_get_value(0x320A);
    t[1] = sensor_reg_get_value(0x320B);
    h = (t[0]<<8)|t[1];
    TRACE(1, "[sensor width=%d, height=%d]", w, h);
}

static int sensor_reset(void)
{
    i2c_write(0x0103, 0x01);
    osDelay(1);

    return 0;
}

static void sensor_soft_sleep_mode(bool enable)
{
    if (enable) {
        i2c_write(0x0100, 0x00);
    } else {
        i2c_write(0x0100, 0x01);
    }
}

void sensor_hdr_mode_enable(bool enable)
{
    uint8_t val = 0;
    uint16_t addr = 0x3220;

    TRACE(1, "[%s]: enable=%d", __func__, enable);
    i2c_read(addr, &val);
    if (enable) {
        val |= (1<<6); //HDR enable
    } else {
        val &= ~(1<<6); //HDR disable
    }
    i2c_write(addr, val);
}

void sensor_hdrc_mode_enable(bool enable)
{
    uint8_t val = 0;
    uint16_t addr = 0x5001;

    TRACE(1, "[%s]: enable=%d", __func__, enable);
    i2c_read(addr, &val);
    if (enable) {
        val &= ~(1<<0); //HDRC enable
    } else {
        val |= (1<<0);  //HDRC disable
    }
    i2c_write(addr, val);
}

static int sensor_get_chip_id(uint8_t cid[])
{
#define CID_H 0x01
#define CID_L 0x32
    int err = 0;
    uint8_t chip_id[2] = {0};

    i2c_read(0x3107, &chip_id[0]);
    i2c_read(0x3108, &chip_id[1]);
    if (cid) {
        cid[0] = chip_id[0];
        cid[1] = chip_id[1];
    }
    TRACE(1, "probe sensor: CHIP ID: %02x-%02x", chip_id[0], chip_id[1]);
    if ((chip_id[0] == CID_H) && (chip_id[1] == CID_L)) {
        TRACE(1, "[%s]: GOOD CAMERA CHIP ID: [%x-%x]", __func__, chip_id[0],chip_id[1]);
    } else {
        TRACE(1, "[%s]: BAD CAMERA CHIP ID: [%x-%x]", __func__, chip_id[0],chip_id[1]);
        err = 1;
    }
    return err;
}

static int sensor_test_mode(int mode, bool enable)
{
    if (enable) {
        i2c_write(0x4501, 0xac);
        i2c_write(0x3902, 0x85);
        i2c_write(0x391d, 0xa8);
        i2c_write(0x3e06, 0x03);
    } else {
        i2c_write(0x4501, 0xa4);
        i2c_write(0x3902, 0xc5);
        i2c_write(0x391d, 0xac);
        i2c_write(0x3e06, 0x00);
    }
    return 0;
}


static void csi_sensor_phy_disable(void)
{
}

static void csi_sensor_phy_enable(void)
{
}

static void sensor_init(void)
{
    camera_device_hw_pin_init();
}

static void sensor_power_on(bool on)
{
    TRACE(0, "[%s]: on=%d", __func__, on);

    if (sens_pwr_status == on) {
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
    int ret = 0;
    uint32_t time;

    time = hal_sys_timer_get();

    TRACE(0, "[%s]", __func__);

    sensor_get_chip_id(NULL);
    sensor_reset();
    sensor_soft_sleep_mode(true);
    sensor_clock_init();

    time = hal_sys_timer_get() - time;
    TRACE(0, "[%s]: okay, cost %d ms", __func__, TICKS_TO_MS(time));
    return ret;
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

static int sensor_start(void)
{
    TRACE(0, "[%s]", __func__);

    sensor_common_init();
    //sensor_hdr_mode_enable(true);
    //sensor_hdrc_mode_enable(true);
#ifdef CAMERA_TEST_MODE_INIT_ON
    sensor_test_mode(0, true);
#endif
    csi_sensor_phy_enable();
    sensor_regs_dump();
    return 0;
}

static int sensor_stop(void)
{
    TRACE(0, "[%s]", __func__);

    csi_sensor_phy_disable();
    sensor_reset();
    return 0;
}

static int sensor_close(void)
{
    TRACE(0, "[%s]", __func__);

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
static int sensor_dummy_bus_write(uint32_t reg_addr, uint32_t val) {return 0;}
static int sensor_dummy_bus_read(uint32_t reg_addr, uint32_t *val) {return 0;}
static int sensor_dummy_test_mode(int mode, bool enable) {return 0;}
static int sensor_dummy_get_config(struct VP_DEVICE_CONFIG_T *cfg) {return 0;}
static int sensor_dummy_set_config(struct VP_DEVICE_CONFIG_T *cfg) {return 0;}
static int sensor_dummy_get_op_flag(void) {return 0;}
static int sensor_dummy_get_bus(struct VP_DEVICE_BUS_T *bus) {return 0;}
static int sensor_dummy_set_bus(struct VP_DEVICE_BUS_T *bus) {return 0;}

static const struct VP_DEVICE_OPS_T sc132gs_dum_ops = {
    .name = "SC132GS_DUMMY",
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
    .op_flag = sensor_dummy_get_op_flag,
    .get_bus = sensor_dummy_get_bus,
    .set_bus = sensor_dummy_set_bus,
};
#endif

static const struct VP_DEVICE_OPS_T sc132gs_ops = {
    .name = "SC132GS",
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

struct VP_DEVICE_OPS_T *sc132gs_sensor_ops_get(void)
{
    const struct VP_DEVICE_OPS_T *ops = &sc132gs_ops;

#ifdef CAMERA_DUMMY_DEVICE
    ops = &sc132gs_dum_ops;
#endif

    return (struct VP_DEVICE_OPS_T *)ops;
}

#endif /* CSI_SENS_SC132GS */
