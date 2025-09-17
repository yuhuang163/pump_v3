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
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
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
#ifdef DSI_SENS_A064
#include "display_drv_a064.h"
#endif

#define DISP_DRV_DBG_EN

#define ASSERT_NUL_PTR() ASSERT(dev_ops != NULL, "[%s]: null ptr", __func__);
#ifdef DISP_DRV_DBG_EN
#define DISP_DRV_DEBUG() TRACE(0, "[%s]: ", __func__);
#else
#define DISP_DRV_DEBUG(...) do{ } while(0)
#endif

static struct VP_DEVICE_OPS_T *dev_ops = NULL;
static struct VP_DEVICE_CONFIG_T *dev_cfg = NULL;

struct VP_DEVICE_OPS_T *display_device_setup(enum VP_DEVICE_T device, struct VP_DEVICE_CONFIG_T *cfg)
{
    if (0) {
#ifdef DSI_SENS_A064
    } else if (device == VP_DEVICE_A064) {
        dev_ops = a064_sensor_ops_get();
        if (dev_ops->init) {
            dev_ops->init();
        }
        TRACE(1, "setup display device %s done", dev_ops->name);
#endif
    } else {
        ASSERT(0, "[%s]: not support display device %d", __func__, device);
    }
    dev_cfg = cfg;
    return dev_ops;
}

void display_device_power_on(void)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    dev_ops->power_on(true);
}

void display_device_power_off(void)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    dev_ops->power_on(false);
}

int display_device_probe(void)
{
    uint8_t chip_id[4] = {0};

    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    dev_ops->get_chip_id(chip_id);
    TRACE(1, "probe display device: CHIP_ID: %x-%x-%x", chip_id[0],chip_id[1],chip_id[2]);

    return 0;
}

int display_device_open(void)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    return dev_ops->open();
}

int display_device_start(void)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    return dev_ops->start();
}

int display_device_stop(void)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    return dev_ops->stop();
}

int display_device_close(void)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    return dev_ops->close();
}

int display_device_reset(void)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    return dev_ops->reset();
}

int display_device_bus_write(uint32_t reg_addr, uint32_t val)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    return dev_ops->bus_write(reg_addr, val);
}

int display_device_bus_read(uint32_t reg_addr, uint32_t *val)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    return dev_ops->bus_read(reg_addr, val);
}

void display_device_test_mode(bool enable, uint8_t value)
{
    ASSERT_NUL_PTR();
    DISP_DRV_DEBUG();

    dev_ops->test_mode(value, enable);
}

void dsi_sensor_setup_in_image_size(uint32_t width, uint32_t height);

void display_device_setup_input_image_size(uint32_t width, uint32_t height)
{
    dsi_sensor_setup_in_image_size(width, height);
}

WEAK void display_get_boot_image_info(uint8_t **img_data, uint32_t *img_size)
{
}


WEAK uint32_t display_load_boot_image_data(uint8_t *dst, uint32_t dst_size)
{
    return 0;
}
