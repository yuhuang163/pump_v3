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

#ifndef __YUV_CONV_H__
#define __YUV_CONV_H__

#ifdef __cplusplus
extern "C" {
#endif

/* YUV422p/YUV420p/YUV444p conversion */

void yuv420_planar_to_yuv422_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv420_planar_to_yuv444_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

void yuv422_planar_to_yuv420_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv422_planar_to_yuv444_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

void yuv444_planar_to_yuv422_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv444_planar_to_yuv420_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

/* RGBA8888 & YUV422p/YUV420p/YUV444p conversion */

void yuv420_planar_to_rgba8888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv422_planar_to_rgba8888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv444_planar_to_rgba8888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

void rgba8888_to_yuv420_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void rgba8888_to_yuv422_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void rgba8888_to_yuv444_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

/* RGB888 & YUV422p/YUV420p/YUV444p conversion */

void yuv420_planar_to_rgb888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv422_planar_to_rgb888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv444_planar_to_rgb888(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

void rgb888_to_yuv420_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void rgb888_to_yuv422_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void rgb888_to_yuv444_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

/* YUV422 planar & packed(VYUY, YUYV) conversion */

void yuv422_vyuy_to_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv422_planar_to_vyuy(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

void yuv422_yuyv_to_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv422_planar_to_yuyv(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

void yuv422_vyuy_to_yuyv(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv422_yuyv_to_vyuy(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv422_vyuy_to_yvyu(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);
void yuv422_vyuy_to_uyvy(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h);

#ifdef __cplusplus
}
#endif
#endif /* __YUV_CONV_H__ */
