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
#ifndef __VIDEO_PUBLIC_H__
#define __VIDEO_PUBLIC_H__

#ifdef __cplusplus
extern "C" {
#endif

enum VP_CHANNEL_MAP_T {
    VP_CHANNEL_MAP_CH0 = (1<<0),
    VP_CHANNEL_MAP_CH1 = (1<<1),

    VP_CHANNEL_MAP_ALL = (0x3),
};

#define VP_CHANNEL_MAP_QTY 2

enum VP_CHANNEL_NUM_T {
    VP_CHANNEL_NUM_NULL = 0,
    VP_CHANNEL_NUM_1 = 1,
    VP_CHANNEL_NUM_2 = 2,
};

enum VP_LAYER_ID_T {
    VP_LAYER_ID_0 = 0,
    VP_LAYER_ID_1 = 1,
    VP_LAYER_ID_2 = 2,
};

enum VP_DEVICE_T {
    VP_DEVICE_NULL = 0,
    VP_DEVICE_A064,
    VP_DEVICE_GC2145,
    VP_DEVICE_SC132GS,
};

enum VP_PIXEL_FMT_T {
    VP_PIXEL_FMT_RGB565  = 0,
    VP_PIXEL_FMT_RGB888  = 1,
    VP_PIXEL_FMT_YUV422  = 2,
    VP_PIXEL_FMT_RGBA888 = 3,
};

struct VP_DEVICE_CONFIG_T {
    uint32_t width;
    uint32_t height;
    enum VP_PIXEL_FMT_T fmt;
    enum VP_CHANNEL_NUM_T channel_num;
    int op_flag;
};

#define CAMERA_DESC_NUM 4

#define CAMERA_DEV_NUM 2
#define CAMERA_DEV_LIST {VP_DEVICE_GC2145, VP_DEVICE_SC132GS}

#define DISPLAY_DEV_NUM 1
#define DISPLAY_DEV_LIST {VP_DEVICE_A064}

enum CAMERA_OP_FLAG_T {
    CAMERA_OP_FLAG_NULL         = 0,
    CAMERA_OP_FLAG_SOFT_ISP     = (1 << 0),
    CAMERA_OP_FLAG_RAW10_TO_YUV = (1 << 1),
    CAMERA_OP_FLAG_CALIB_PHY    = (1 << 2),

    CAMERA_OP_FLAG_ALL          = 0x7,
};

enum VP_DEVICE_ID_T {
    VP_DEVICE_ID_0,
    VP_DEVICE_ID_1,
    VP_DEVICE_ID_2,
    VP_DEVICE_ID_3,
};

struct VP_DEVICE_BUS_T {
    uint8_t user;
    uint8_t id;
    uint8_t addr;
    uint32_t speed;
};

struct VP_DEVICE_OPS_T {
    const char *name;
    void (*init)(void);
    void (*power_on)(bool on);
    int (*get_chip_id)(uint8_t chip_id[]);
    int (*open)(void);
    int (*start)(void);
    int (*stop)(void);
    int (*close)(void);
    int (*reset)(void);
    int (*bus_write)(uint32_t reg, uint32_t addr);
    int (*bus_read)(uint32_t reg, uint32_t *val);
    int (*test_mode)(int mode, bool enable);
    int (*get_config)(struct VP_DEVICE_CONFIG_T *cfg);
    int (*set_config)(struct VP_DEVICE_CONFIG_T *cfg);
    int (*get_op_flag)(void);
    int (*get_bus)(struct VP_DEVICE_BUS_T *bus);
    int (*set_bus)(struct VP_DEVICE_BUS_T *bus);
};

#ifdef __cplusplus
}
#endif

#endif
