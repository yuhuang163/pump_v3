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
#ifndef __CAMERA_DRV_H__
#define __CAMERA_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "video_public.h"

/* open camera device driver */
int camera_device_open(void);

/* close camera device driver */
int camera_device_close(void);

/* probe camera device if it's connected
 *
 * cam_devce: ptr to camera device type
 * bus: ptr to camera bus
 *
 * return: success(true) or failure(false)
 */
bool camera_device_probe(enum VP_DEVICE_T *cam_device, struct VP_DEVICE_BUS_T *bus);

/* get camera device configuration
 *
 * cam_devce: camera device type
 * cfg: ptr to camera configuration will be written;
 *
 * return: success(0) or failure(!0)
 */

int camera_device_get_config(enum VP_DEVICE_T cam_device, struct VP_DEVICE_CONFIG_T *cfg);

/* set camera device configuration
 *
 * cam_devce: camera device type
 * cfg: ptr to camera configuration;
 *
 * return: success(0) or failure(!0)
 */
int camera_device_set_config(enum VP_DEVICE_T cam_device, struct VP_DEVICE_CONFIG_T *cfg);

/* open camera stream
 *
 * dev: device type id
 * bus: ptr to camera bus
 * cfg: device configuration
 *
 * return: device descripter id (>= 0) or failure( < 0)
 */
int camera_device_stream_open(enum VP_DEVICE_T dev, struct VP_DEVICE_BUS_T *bus,
    struct VP_DEVICE_CONFIG_T *cfg);

/* close camera stream
 *
 * desc_id: device descripter id
 *
 * return: success(0) or failure(!0)
 */
int camera_device_stream_close(int desc_id);

/* config camera stream
 *
 * desc_id: device descripter id
 * cfg: device configuration
 *
 * return: success(0) or failure(!0)
 */
int camera_device_stream_setup(int desc_id, struct VP_DEVICE_CONFIG_T *cfg);

/* start camera stream
 *
 * desc_id: device descripter id
 *
 * return: success(0) or failure(!0)
 */
int camera_device_stream_start(int desc_id);

/* stop camera stream
 *
 * desc_id: device descripter id
 *
 * return: success(0) or failure(!0)
 */
int camera_device_stream_stop(int desc_id);

/* write data to device regiseter
 *
 * dev: device type id
 * reg_addr: the register address
 * val: the register value
 *
 */
int camera_device_bus_write(int desc_id, uint32_t reg_addr, uint32_t val);

/* read data from device regiseter
 *
 * dev: device type id
 * reg_addr: the register address
 * *val: the register value pointer
 *
 */
int camera_device_bus_read(int desc_id, uint32_t reg_addr, uint32_t *val);

/* camera test mode enable/disable
 *
 * dev: device type id
 * enable: true/false
 * p0/p1/p2: parameters for test mode
 *
 */
void camera_device_test_mode(int desc_id, bool enable, uint8_t p0, uint8_t p1, uint8_t p2);

enum CAMERA_I2C_USER_T {
    CAMERA_I2C_USER_0,
    CAMERA_I2C_USER_1,
    CAMERA_I2C_USER_2,
};

enum CAMERA_I2C_ID_T {
    CAMERA_I2C_ID_0,
    CAMERA_I2C_ID_1,
    CAMERA_I2C_ID_2,

    CAMERA_I2C_ID_QTY,
};

enum CAMERA_I2C_SPEED_T {
    CAMERA_I2C_SPEED_100KHZ = 100000,
    CAMERA_I2C_SPEED_200KHZ = 200000,
    CAMERA_I2C_SPEED_300KHZ = 300000,
    CAMERA_I2C_SPEED_400KHZ = 400000,
};

enum CAMERA_I2C_BUS_ID_T {
    CAEMRA_I2C_BUS_ID_0,
    CAEMRA_I2C_BUS_ID_1,
    CAMERA_I2C_BUS_ID_QTY,
};

int camera_device_hw_i2c_open(enum CAMERA_I2C_USER_T user, enum CAMERA_I2C_ID_T id, uint32_t speed);

int camera_device_hw_i2c_close(enum CAMERA_I2C_USER_T user, enum CAMERA_I2C_ID_T id);

int camera_device_hw_i2c_recv(enum CAMERA_I2C_USER_T user, enum CAMERA_I2C_ID_T id,
    uint8_t addr, uint8_t *txbuf, uint16_t txlen, uint8_t *rxbuf, uint16_t rxlen);

int camera_device_hw_i2c_send(enum CAMERA_I2C_USER_T user, enum CAMERA_I2C_ID_T id,
    uint8_t addr, uint8_t *txbuf, uint16_t txlen);

void camera_device_hw_pin_init(void);

void camera_device_hw_power_on(enum VP_DEVICE_ID_T id, enum VP_DEVICE_T device);

void camera_device_hw_power_off(enum VP_DEVICE_ID_T id, enum VP_DEVICE_T device);

#include "camera_dump.h"

#ifdef __cplusplus
}
#endif

#endif
