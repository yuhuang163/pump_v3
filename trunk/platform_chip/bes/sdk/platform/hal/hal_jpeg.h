/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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

#ifndef __HAL_JPEG_H__
#define __HAL_JPEG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum HAL_JPEG_ID_T {
    HAL_JPEG_ID_0 = 0,
#ifdef JPEG1_BASE
    HAL_JPEG_ID_1 = 1,
#endif

    HAL_JPEG_ID_QTY,
};

enum HAL_JPEG_RET_T {
    HAL_JPEG_OK = 0,
    HAL_JPEG_ERR = 1,
    HAL_JPEG_NOT_SUPPORT = 2,
    HAL_JPEG_PIX_DMA_ERR = 3,
    HAL_JPEG_ECS_DMA_ERR = 4,
    HAL_JPEG_NOT_OPENED = 5,
};

enum HAL_JPEG_MODE_T {
    HAL_JPEG_ENC_MODE = 0,
    HAL_JPEG_DEC_MODE = 1,
};

enum HAL_JPEG_FORMAT_T {
    HAL_JPEG_YUV422 = 0,
    HAL_JPEG_YUV420,
    HAL_JPEG_YUV444,
};

struct HAL_JPEG_CONFIG_T {
    enum HAL_JPEG_MODE_T mode;
    enum HAL_JPEG_FORMAT_T format;

    /* Picture size for encode/decode */
    uint16_t width;
    uint16_t height;

    /* Swap YU or UV of the YUV422 pixel data for decode/encode;
     * Default:           [V0,Y0,U0,Y1];
     * SWAP_UV:           [U0,Y0,V0,Y1];
     * SWAP_YU:           [Y0,U0,Y1,V0];
     * SWAP_UV & SWAP_YU: [Y0,V0,Y1,U0];
     */
    bool swap_yu;
    bool swap_uv;

    /* MCU numbers in restart interval */
    int rst_nmcu;

    /* QTable/Htable selection for color components */
    uint8_t qt[4];
    uint8_t hat[4];
    uint8_t hdt[4];

    /* Input for decode, output for encode */
    void *ecs;
    uint32_t ecs_size;

    /* Input for encode, output for decode */
    void *pix;
    uint32_t pix_size;

    /* Quantization table for encode/decode */
    void *qtable;
    uint32_t qtable_size;

    /* Huffman base/min/symb data for decode */
    void *hbase;
    uint32_t hbase_size;

    void *hmin;
    uint32_t hmin_size;

    void *hsymb;
    uint32_t hsymb_size;

    /* Huffman table for encode */
    void *hetable;
    uint32_t hetable_size;

    /* Callback for work done */
    void (*done_cb)(enum HAL_JPEG_ID_T id);
};

/**
 * @brief Enable JPEG Codec and DMA engine.
 *
 * @param id JPEG codec module id.
 */
int hal_jpeg_open(enum HAL_JPEG_ID_T id);

/**
 * @brief Disable JPEG Codec and DMA engine.
 *
 * @param id JPEG codec module id.
 */
int hal_jpeg_close(enum HAL_JPEG_ID_T id);

/**
 * @brief Start JPEG decoding/encoding, and DMA data transfer.
 *
 * @param id JPEG codec module id.
 * @param cfg JPEG decoding/encoding configuration.
 */
int hal_jpeg_start(enum HAL_JPEG_ID_T id, struct HAL_JPEG_CONFIG_T *cfg);

/**
 * @brief Finish JPEG decoding/encoding.
 *
 * @param id JPEG codec module id.
 */
int hal_jpeg_stop(enum HAL_JPEG_ID_T id);

/**
 * @brief Check JPEG Codec working status.
 *
 * @param id JPEG codec module id.
 */
bool hal_jpeg_busy(enum HAL_JPEG_ID_T id);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_JPEG_H__ */

