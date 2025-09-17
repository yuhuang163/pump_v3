/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "plat_addr_map.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "stdio.h"
#include "mcu_audio.h"
#include "zbar.h"
#include "stdlib.h"
#include "zbar_detector.h"
#include "vplayer_app_common.h"
#include "ap_qrcode_app.h"
#include "qrcode_gui.h"

uint8_t *gray_img = NULL;

qrcode_result_t qrcode_result;
void zbar_init()
{
    printf("zbar init\n");
}

void qrcode_img_copy(struct qrcode_img_cfg *img)
{
#define RAW10_YUV(v) (((v) >> 2))
    uint8_t *buf = (uint8_t *)img->addr;
    uint32_t len = img->width * img->height * 2;
    uint32_t cost_time = hal_sys_timer_get();
    uint32_t gray_img_len = len / 2;
    if (!gray_img) {
        gray_img = malloc(gray_img_len);
        memset(gray_img, 0, gray_img_len);
    }

    uint8_t *pd = (uint8_t *)(REMAP_C_ADDR(gray_img));
    uint16_t *ps = (uint16_t *)(REMAP_C_ADDR(buf));
    uint32_t blk = gray_img_len / 8;
    uint32_t lst = (gray_img_len) % 8;
    uint32_t i;

    if (blk > 0) {
        for (i = 0; i < blk; i++) {
            pd[0] = (uint8_t)((RAW10_YUV(ps[0])) & 0xff);
            pd[1] = (uint8_t)((RAW10_YUV(ps[1])) & 0xff);
            pd[2] = (uint8_t)((RAW10_YUV(ps[2])) & 0xff);
            pd[3] = (uint8_t)((RAW10_YUV(ps[3])) & 0xff);
            pd[4] = (uint8_t)((RAW10_YUV(ps[4])) & 0xff);
            pd[5] = (uint8_t)((RAW10_YUV(ps[5])) & 0xff);
            pd[6] = (uint8_t)((RAW10_YUV(ps[6])) & 0xff);
            pd[7] = (uint8_t)((RAW10_YUV(ps[7])) & 0xff);
            pd += 8;
            ps += 8;
        }
    }
    if (lst > 0) {
        for (i = 0; i < lst; i++) {
            pd[i] = (uint8_t)((RAW10_YUV(ps[i])) & 0xff);
        }
    }

    TRACE(1, "[%s]: img_copy_cost %d ms", __func__, TICKS_TO_MS(hal_sys_timer_get() - cost_time));
}

int zbar_detect(struct qrcode_img_cfg *img)
{
    zbar_image_scanner_t *scanner = NULL;
    scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    uint32_t width = img->width;
    uint32_t height = img->height;
    if (!scanner || !gray_img) {
        printf("zbar not init or no image\n");
        return 0;
    }
    printf("**********zbar start*********\n*");
    zbar_image_t *image = zbar_image_create();
    // zbar only support gray image，format can be "GREY" or "Y800"
    zbar_image_set_format(image, *(int *)"Y800");
    zbar_image_set_size(image, width, height);
    // for single channel image, image_length must be width*height
    zbar_image_set_data(image, gray_img, width * height, zbar_image_free_data);

    uint32_t cost_time = hal_sys_timer_get();
    int n = zbar_scan_image(scanner, image);
    TRACE(1, "[%s]: zbar_scan_cost %d ms", __func__, TICKS_TO_MS(hal_sys_timer_get() - cost_time));
    if (n == 0 || n == -1) {
        printf("no symbols were found or an error occurs\n");
        zbar_image_scanner_destroy(scanner);
        if (image) {
            free(image);
            image = NULL;
        }
        return 0;
    }

    memset(&qrcode_result, 0x0, sizeof(qrcode_result));
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    zbar_symbol_type_t symbol_type = zbar_symbol_get_type(symbol);
    const char *symbol_type_name = zbar_get_symbol_name(symbol_type);
    memcpy(qrcode_result.type, symbol_type_name, strlen(symbol_type_name));
    memcpy(qrcode_result.data, zbar_symbol_get_data(symbol), zbar_symbol_get_data_length(symbol));
    // printf("decoded %s symbol \"%s\"\n", qrcode_result.type, qrcode_result.data);
    qrcode_result.location_count = zbar_symbol_get_loc_size(symbol);
    for (int i = 0; i < qrcode_result.location_count; i++) {
        qrcode_result.location_x[i] = (int32_t)zbar_symbol_get_loc_x(symbol, i);
        qrcode_result.location_y[i] = (int32_t)zbar_symbol_get_loc_y(symbol, i);
        // printf("location %d: x %d, y %d \n", i, qrcode_result.location_x[i], qrcode_result.location_x[i]);
    }
    zbar_image_scanner_destroy(scanner);
    if (image) {
        free(image);
        image = NULL;
    }
    printf("**********zbar end**********\n");
    return 1;
}

// 未使用
void zbar_destory()
{

}
