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
#ifndef __CAMERA_DUMP_H__
#define __CAMERA_DUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CAMERA_DATA_CHECK_CALLBACK_T)(int err_cnt, int chk_cnt, int seq);

void camera_test_data_check_init(uint32_t width, uint32_t height, uint32_t pixel_size,
    CAMERA_DATA_CHECK_CALLBACK_T callback, bool oneshot);

void camera_test_data_check_enable(bool enable);

int camera_test_data_check_run(uint8_t *buf, uint32_t len);

void camera_data_dump_trap(void);

void camera_data_dump(uint8_t *buf, uint32_t len, uint32_t pix_bits);

void camera_data_dump2(uint8_t *buf, uint32_t len, uint32_t pix_bits);

int camera_data_raw10_to_yuv422(uint32_t dst, uint32_t src, uint32_t size);

int camera_data_raw10_to_yuv420(uint32_t dst, uint32_t src, uint32_t size);

int camera_data_memcpy(uint8_t *dst, uint8_t *src, uint32_t size);

int camera_data_memcpy_dma(uint8_t *dst, uint8_t *src, uint32_t size);

int camera_data_clean(uint8_t *src, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
