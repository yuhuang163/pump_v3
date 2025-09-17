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
#ifndef __CSI_DSI_CONF_H__
#define __CSI_DSI_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* CSI DSI DATA BUFFER CONFIG */

#if (CAMERA_IMAGE_WIDTH == 1080) && (CAMERA_IMAGE_HEIGHT == 1280)
#define CSI_DATA_BUF_SIZE (CAMERA_IMAGE_WIDTH * CAMERA_IMAGE_HEIGHT * 2 * 2)
#else
#define CSI_DATA_BUF_SIZE (0x200000)
#endif

#define DSI_DATA_BUF_SIZE (0x200000)
#ifdef PSRAMUHS_ENABLE
#define CSI_DSI_BUF_BASE   (PSRAMUHS_NC_BASE)
#define CSI_DSI_BUF_END    (PSRAMUHS_NC_BASE + DSP_PSRAMUHS_SIZE)
#define DSI_DATA_BUF_BASE  (CSI_DSI_BUF_END - DSI_DATA_BUF_SIZE)
#define CSI_DATA_BUF_BASE  (DSI_DATA_BUF_BASE - CSI_DATA_BUF_SIZE)
#define CSI_DATA_BUF_ADDR  (CSI_DATA_BUF_BASE)
#define DSI_DATA_BUF_ADDR  (DSI_DATA_BUF_BASE)

#else /* PSRAMUHS_ENABLE */
#define CSI_DATA_BUF_ADDR  (DSP_RAM_BASE)
#define DSI_DATA_BUF_ADDR  (DSP_RAM_BASE)
#endif

/* MEDIA HEAP BUFFER */
#if !defined(CHIP_ROLE_DSP) && !defined(CHIP_ROLE_AP)
#define MEDIA_HEAP_RAW_SIZE    (CSI_DATA_BUF_SIZE + DSI_DATA_BUF_SIZE)
#elif defined(CHIP_ROLE_DSP) && !defined(CHIP_ROLE_AP)
#define MEDIA_HEAP_RAW_SIZE (0x240000)
#elif defined(CHIP_ROLE_AP) && !defined(CHIP_ROLE_DSP)
#define MEDIA_HEAP_RAW_SIZE (0x100000)
#else
#error "both CHIP_ROLE_AP and CHIP_ROLE_DSP are defined !"
#endif

#ifndef MEDIA_HEAP_SIZE
#define MEDIA_HEAP_SIZE    MEDIA_HEAP_RAW_SIZE
#endif

#ifndef MEDIA_HEAP_ADDR
#define MEDIA_HEAP_ADDR    (CSI_DATA_BUF_BASE)
#endif

/* CSI SENSOR PIN CONFIG */
#define CSI_SENS_PDN_PIN   (HAL_GPIO_PIN_P2_4)
#define CSI_SENS_RSTN_PIN  (HAL_GPIO_PIN_P2_5)
#define CSI_SENS_MCLK_PIN  (HAL_GPIO_PIN_P3_4)

/* CSI SYNC PIN CONFIG */
#define CSI_SYNC_OUT_PIN HAL_IOMUX_PIN_P3_0
#define CSI_SYNC_IN_PIN  HAL_IOMUX_PIN_P3_1

/* CSI SENSOR I2C CONFIG */
/* CSI SENSOR I2C CONFIG */

#ifdef CAMERA_IMAGE_WIDTH
#define CSI_IMG_PIXEL_WIDTH CAMERA_IMAGE_WIDTH
#else
#define CSI_IMG_PIXEL_WIDTH 800
#endif
#ifdef CAMERA_IMAGE_HEIGHT
#define CSI_IMG_PIXEL_HEIGHT CAMERA_IMAGE_HEIGHT
#else
#define CSI_IMG_PIXEL_HEIGHT 600
#endif

#ifdef __cplusplus
}
#endif

#endif
