/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __BAJDEC_H__
#define __BAJDEC_H__

enum BAJDEC_RET_T {
    BAJ_OK              = 0,
    BAJ_DATA_ERR        = -1,
    BAJ_BUFF_ERR        = -2,
    BAJ_ALPHA_ERR       = -3,
    BAJ_JPEG_ERR        = -4,
    BAJ_CONV_ERR        = -5,
};

/**
 * @brief Get BAJ file info.
 *
 * @param data              [in]  BAJ file data.
 * @param data_size         [in]  BAJ file size.
 * @param width             [out] Image width.
 * @param height            [out] Image height.
 * @param aligned_width     [out] Image aligned width.
 * @param aligned_height    [out] Image aligned height.
 */
int baj_get_info(uint8_t *data, uint32_t data_size, int *width, int *height,
                 int *aligned_width, int *aligned_height);

/**
 * @brief Decode BAJ file to RGBA pixel data.
 *
 * @param data       [in]  BAJ file data.
 * @param data_size  [in]  BAJ file size.
 * @param dstbuf     [in]  Target RGBA pixel buffer.
 * @param tmpbuf     [in]  Temp buffer needed by decoding.
 * @param tempbuf_sz [in]  Temp buffer size, >= aligned_width * aligned_height * 7.
 */
int baj_decode(uint8_t *data, uint32_t data_size, uint8_t *dstbuf,
                 uint8_t *tmpbuf, uint32_t tmpbuf_sz);

#endif /* __BAJDEC_H__ */
