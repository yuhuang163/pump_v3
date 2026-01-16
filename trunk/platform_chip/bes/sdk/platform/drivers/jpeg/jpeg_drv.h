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
#ifndef __JPEG_DRV_H__
#define __JPEG_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

enum JPEG_RESULT {
    JPEG_OK = 0,
    JPEG_FAILED,
    JPEG_TIMEOUT,
    JPEG_FILE_ERR,
    JPEG_HAL_ERR,
    JPEG_NOT_SUPPORT,
    JPEG_BUF_UNALIGNED,
    JPEG_BUF_SMALL,
    JPEG_BUSY,
};

enum JPEG_FORMAT {
    JPEG_YUV422 = 0,
    JPEG_YUV420,
    JPEG_YUV444,
};

enum PIX_FORMAT {
    PIX_YUV422_VYUY = 0,
    PIX_YUV422_YUYV,
    PIX_YUV422_UYVY,
    PIX_YUV422_YVYU,
};

struct jpeg_handle_t {
    /* The orignal jpeg picture width and height.
     * Decode: set by jpeg_decode() function, parsed from jpeg file.
     * Encode: no use.
     */
    uint16_t jpeg_width;
    uint16_t jpeg_height;

    /* Jpeg picture format, YUV420/422/444.
     * Decode: set by jpeg_decode() function, parsed from jepg file.
     * Encode: set by caller of jpeg_encode().
     */
    enum JPEG_FORMAT jpeg_fmt;

    /* The raw pixel image width and height.
     * Alignment requirements:
     *   YUV420 - 16 pixels;
     *   YUV444 - 8 pixels;
     *   YUV422 - 16 pixels for width, 8 pixels for height;
     * Decode: set by jpeg_decode() function, aligned from jpeg picture width & height.
     * Encode: set by caller of jpeg_encode().
     */
    uint16_t width;
    uint16_t height;

    /* The raw pixel image format, YUV422 VYUY/YUYV/UYVY/YVYU.
     * Decode: set by caller of jpeg_decode();
     * Encode: set by caller of jpeg_encode();
     */
    enum PIX_FORMAT pix_fmt;

    /* Pixel data buffer pointer, store pixel image data.
     * Decode: set by caller of jpeg_decode();
     * Encode: set by caller of jpeg_encode();
     */
    uint8_t *pix_data;

    /* Pixel image data size, or data buffer size for decoding.
     * Alignment requirements: 4 bytes.
     * Decode: set buffer size by caller, after decoding, it will be updated to image data size.
     * Encode: set pixel image data size by caller.
     */
    uint32_t pix_size;

    /* Jpeg data buffer pointer, store encoded jpeg file data.
     * Alignment requirements: 4 bytes.
     * Decode: set by caller of jpeg_decode();
     * Encode: set by caller of jpeg_encode();
     */
    uint8_t *jpeg_data;

    /* Jpeg file size, or jpeg buffer size for encoding.
     * Decode: set jpeg file size by caller.
     * Encode: set buffer size by caller, after encoding, it will be updated to jpeg file size.
     */
    uint32_t jpeg_size;

    /* Temporary buffer for decoding & encoding, store ECS data.
     * Alignment requirements: 4 bytes.
     * The buffer must has enough space, better equal to jpeg file size.
     */
    uint8_t *ecs_buf;
    uint32_t ecs_bufsz;
};

/**
 * @brief jpeg driver initialize;
 */
void jpeg_init(void);

/**
 * @brief jpeg driver de-initialize;
 */
void jpeg_exit(void);

/**
 * @brief decode jpeg file to pixel data;
 *
 * @param handle decoding handle pointer;
 *
 * @return 0 for OK, other for failed;
 */
int jpeg_decode(struct jpeg_handle_t *handle);

/**
 * @brief encode pixel data to a jpeg file;
 *
 * @param handle encoding handle pointer;
 *
 * @return 0 for OK, other for failed;
 */
int jpeg_encode(struct jpeg_handle_t *handle);


#ifdef __cplusplus
}
#endif
#endif /* __JPEG_DRV_H__ */
