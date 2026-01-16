/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#include "plat_types.h"
#include "lvgl.h"
#include "bes_tp_hal.h"

static void touchpad_init(void);
static void touchpad_exit(lv_event_t *event);
static void touchpad_read(lv_indev_t * indev, lv_indev_data_t * data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(int32_t * x, int32_t * y);

void lv_hal_indev_init(void)
{
    touchpad_init();

    lv_indev_t *indev_touchpad = lv_indev_create();
    lv_indev_set_driver_data(indev_touchpad, NULL);
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, touchpad_read);
    lv_indev_add_event_cb(indev_touchpad, touchpad_exit, LV_EVENT_DELETE, indev_touchpad);
}

static void touchpad_init(void)
{
    bes_tp_hal_init();
}

static void touchpad_exit(lv_event_t *event)
{
    bes_tp_hal_exit();
}

static void touchpad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    static int32_t last_x = 0;
    static int32_t last_y = 0;

    if(touchpad_is_pressed()) {
        touchpad_get_xy(&last_x, &last_y);
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->point.x = last_x;
    data->point.y = last_y;
}

static bool touchpad_is_pressed(void)
{
    return bes_tp_is_pressed();
}

static void touchpad_get_xy(int32_t * x, int32_t * y)
{
    bes_tp_get_xy(x, y);
}
