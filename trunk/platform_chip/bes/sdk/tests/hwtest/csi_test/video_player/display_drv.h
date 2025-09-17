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
#ifndef __DISPLAY_DRV_H__
#define __DISPLAY_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "video_player.h"

struct VP_DEVICE_OPS_T *display_device_setup(enum VP_DEVICE_T device, struct VP_DEVICE_CONFIG_T *cfg);

void display_device_power_on(void);

void display_device_power_off(void);

int display_device_probe(void);

int display_device_open(void);

int display_device_start(void);

int display_device_stop(void);

int display_device_close(void);

int display_device_reset(void);

int display_device_bus_write(uint32_t reg_addr, uint32_t val);

int display_device_bus_read(uint32_t reg_addr, uint32_t *val);

void display_device_test_mode(bool enable);

void display_device_test_mode_set_value(uint8_t value);

void display_device_setup_input_image_size(uint32_t width, uint32_t height);

void display_get_boot_image_info(uint8_t **img_data, uint32_t *img_size);

uint32_t display_load_boot_image_data(uint8_t *dst, uint32_t dst_size);

#ifdef __cplusplus
}
#endif

#endif
