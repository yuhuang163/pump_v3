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
#ifndef __PNGDEC_H
#define __PNGDEC_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "plat_types.h"
#include "hal_png.h"

/*
 * Pre-processor Definitions
 */
#define  INVALID_COMPRESS_MODE  -1
#define  INVALID_COLOR_TYPE     -2
#define  INVALID_COLOR_FMT      -3
#define  NO_MEMORY_SPACE        -4
#define  PNGDEC_DEC_ERROR       -5
#define  INVALID_IMAGE_SIZE     -6

/*
 * Public Types
 */
enum PNG_DEC_PIX_FMT_T {
    PNGDEC_PIX_FMT_RGB565,
    PNGDEC_PIX_FMT_ARGB565,
    PNGDEC_PIX_FMT_RGBA565,
    PNGDEC_PIX_FMT_RGB888,
    PNGDEC_PIX_FMT_ARGB888,
    PNGDEC_PIX_FMT_RGBA888
};

struct png_file_data {
    uint8_t *data;
    uint32_t size;
};

struct dma2d_pngfb {
    uint8_t lid;
    enum PNG_DEC_PIX_FMT_T cfmt;  // must be PNGDEC_PIX_FMT_RGBA888
};

/*
   png decoder can send the output pixel data to a sub rect area of a dest image buffer
          dest image buffer
         |-------------------|
         |    sub rect area  |
         |     ---width----  |
         |     |<-stride->|  |
         |     |      height |
         |     |          |  |
         |     ------------  |
         |<------pitch------>|
         |-------------------|
*/

struct png_pix_buffer {
    uint8_t *data;          //start mem address of the sub rect , allocated by pixbufalloc
    uint16_t width;         //pngimage witch
    uint16_t height;        //pngimage height
    uint16_t stride;        //stride of pngimage
    uint16_t pitch;         //pitch of dest image buffer,
                            //valid value must be not less than the pngimage stride
    uint16_t trigger_lines;  //caller set
    enum PNG_DEC_PIX_FMT_T fmt; //caller set
    bool globle_alpha;      //caller set
    uint8_t alpha;          //caller set
    struct dma2d_pngfb *dma2dfb; //output to layer path of dma2d
    void (*dma2d_start)(void);
    void *(*pixbufalloc)(uint32_t size, uint8_t aligns);
    void (*trigger_lines_handler)(void);
};

struct pngdec {
   struct png_decoder decoder;
};

/**
 * pngdec_initialize - Initialize the pngdec controller
 *
 * Initialize the pngdec controller
 *
 * Returns: OK - On success, an error if initializing failed.
 */

int pngdec_initialize(void);

/*
 * pngdec driver api
 */

/**
 * pngdec -   decode png file into image pixes
 * @pngfile : png file data buffer
 * @dest    : png pixes data buffer
 *
 * Returns: OK on success ,-errno on failure.
 */

int pngdec(struct png_file_data* pngfile, struct png_pix_buffer* dest);


#ifdef __cplusplus
}
#endif

#endif /* __ARCH_ARM_SRC_PNGDEC_H */
