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

#include "hal_trace.h"
#include "check_raw_image.h"

uint32_t argb888torgba888(uint32_t c)
{
    uint8_t a = (c & 0xff);
    uint8_t r = (c & 0xff00) >> 8;
    uint8_t g = (c & 0xff0000) >> 16;
    uint8_t b = (c & 0xff000000) >> 24;

    return (r)|(g << 8)|(b << 16)|(a << 24);
}
uint16_t rgba888torgb565(uint32_t c)
{
    uint8_t r = (c & 0x00f8) >> 3;
    uint8_t g = (c & 0xfc00) >> 10;
    uint8_t b = (c & 0xf80000) >> 19;
    return r|(g << 5)|(b << 11);
}

uint32_t rgba888torgb565a(uint32_t c)
{
    uint8_t r = (c & 0x00f8) >> 3;
    uint8_t g = (c & 0xfc00) >> 10;
    uint8_t b = (c & 0xf80000) >> 19;
    uint8_t a = (c & 0xff000000) >> 24;

    return r|(g << 5)|(b << 11)|(a <<16);
}

uint32_t rgba888toargb565(uint32_t c)
{
    uint8_t r = (c & 0x00f8) >> 3;
    uint8_t g = (c & 0xfc00) >> 10;
    uint8_t b = (c & 0xf80000) >> 19;
    uint8_t a = (c & 0xff000000) >> 24;

    return (r << 8)|(g << 13)|(b << 19)|(a);
}

#define COLOR_ABS(x) ((x) > 0 ? (x) : (-(x)))


int rgba8888_equal(uint32_t c1, uint32_t c2 )
{
    int ret = 0;
    uint8_t r1 = (c1 & 0x00ff);
    uint8_t r2 = (c2 & 0x00ff);
    uint8_t g1 = (c1 & 0xff00) >> 8;
    uint8_t g2 = (c2 & 0xff00) >> 8;
    uint8_t b1 = (c1 & 0xff0000) >> 16;
    uint8_t b2 = (c2 & 0xff0000) >> 16;
    uint8_t a1 = (c1 & 0xff000000) >> 24;
    uint8_t a2 = (c2 & 0xff000000) >> 24;

    ret = (COLOR_ABS((r1 -r2)) > 2) || (COLOR_ABS((b1 -b2)) > 2) ||
          (COLOR_ABS((g1 -g2)) > 2) || (COLOR_ABS((a1 -a2)) > 2);
    return !ret;
}

int rgb565_equal(uint16_t c1, uint16_t c2 )
{
    int ret = 0;
    uint8_t r1 = (c1 & 0x001f);
    uint8_t r2 = (c2 & 0x001f);
    uint8_t g1 = (c1 & 0x07e0) >> 5;
    uint8_t g2 = (c2 & 0x07e0) >> 5;
    uint8_t b1 = (c1 & 0xf800) >> 11;
    uint8_t b2 = (c2 & 0xf800) >> 11;
    ret = (COLOR_ABS((r1 -r2)) > 4) || (COLOR_ABS((b1 -b2)) > 4) ||
          (COLOR_ABS((g1 -g2)) > 4) ;
    return !ret;
}

int rgba565_equal(uint32_t c1, uint32_t c2)
{
    int ret = 0;
    uint8_t r1 = (c1 & 0x001f);
    uint8_t r2 = (c2 & 0x001f);
    uint8_t g1 = (c1 & 0x07e0) >> 5;
    uint8_t g2 = (c2 & 0x07e0) >> 5;
    uint8_t b1 = (c1 & 0xf800) >> 11;
    uint8_t b2 = (c2 & 0xf800) >> 11;
    uint8_t a1 = (c1 & 0x00ff0000) >> 16;
    uint8_t a2 = (c2 & 0x00ff0000) >> 16;
    ret = (COLOR_ABS((r1 -r2)) > 4) || (COLOR_ABS((b1 -b2)) > 4) ||
          (COLOR_ABS((g1 -g2)) > 4) ||(a1 != a2);
    return !ret;
}

int argb565_equal(uint32_t c1, uint32_t c2)
{
    int ret = 0;
    uint8_t r1 = (c1 & 0x001f00) >> 8;
    uint8_t r2 = (c2 & 0x001f00) >> 8;
    uint8_t g1 = (c1 & 0x07e000) >> 13;
    uint8_t g2 = (c2 & 0x07e000) >> 13;
    uint8_t b1 = (c1 & 0xf80000) >> 19;
    uint8_t b2 = (c2 & 0xf80000) >> 19;
    uint8_t a1 = (c1 & 0xff);
    uint8_t a2 = (c2 & 0xff);
    ret = (COLOR_ABS((r1 -r2)) > 4) || (COLOR_ABS((b1 -b2)) > 4) ||
          (COLOR_ABS((g1 -g2)) > 4) ||(a1 != a2);
    return !ret;
}


int check_test_result(raw_image_t *buff,uint32_t *src_rgba8888)
{
    uint32_t *p0 = src_rgba8888;
    uint32_t *p_bpp32;
    uint8_t  *p_bpp8;
    uint16_t *p_bpp16;
    uint32_t  color = 0;

    if(buff->fmt == IMAGE_RGBA8888) {
        p_bpp32 = (uint32_t*)buff->memory;
        for(int i = 0; i < buff->width * buff->height; i++) {
            color = *p_bpp32++;
           if (!rgba8888_equal(*p0++, color)) {
              return -1;
           }
        }
    }

    if(buff->fmt == IMAGE_ARGB8888) {
        p0 = src_rgba8888;
        p_bpp32 = (uint32_t*)buff->memory;
        for(int i = 0; i < buff->width * buff->height; i++) {
            color = *p_bpp32++;
            color = argb888torgba888(color);
           if (!rgba8888_equal(*p0++, color)) {
              return -1;
           }
        }
    }

    if(buff->fmt == IMAGE_RGB565) {
        p0 = src_rgba8888;
        p_bpp16 = (uint16_t*)buff->memory;
        for(int i = 0; i < buff->width * buff->height; i++) {
            color = *p0++;
            color = rgba888torgb565(color);
           if (!rgb565_equal(*p_bpp16++, 0xffff&color)) {
              TRACE(0,"rgb565_equal i:%d |%x|%x|",i, *(p_bpp16 - 1 ), 0xffff&color);
              return -1;
           }
        }
    }

    if(buff->fmt == IMAGE_RGBA5658) {
        p0 = src_rgba8888;
        p_bpp8 = (uint8_t*)buff->memory;
        for(int i = 0; i < buff->width * buff->height; i++) {
            color = *p0++;
            color = rgba888torgb565a(color);

            if (!buff->globle_alpha) {
                if (!rgba565_equal((0x00ffff&(p_bpp8[0]|(p_bpp8[1] << 8)|p_bpp8[2] << 16)), 0x00ffff&color)) {
                   TRACE(0,"rgba565_equal i:%d |%x|%x|%x",i, (0xffffff&(p_bpp8[0]|(p_bpp8[1] << 8)|p_bpp8[2] << 16)), 0xffffff&color,
                   *(p0 -1));
                   //return -1;
                }
            } else {
                color &=~(0xff0000);
                color |= (buff->alpha) << 16;
                if (!rgba565_equal((p_bpp8[0]|(p_bpp8[1] << 8)|(p_bpp8[2] << 16)), 0xffffff&color)) {
                   return -1;
                }
            }
            p_bpp8 += 3;
        }
    }

    if(buff->fmt == IMAGE_ARGB8565) {
        p0 = src_rgba8888;
        p_bpp8 = (uint8_t*)buff->memory;
        for(int i = 0; i < buff->width * buff->height; i++) {
            color = *p0++;
            color = rgba888toargb565(color);

            if (!buff->globle_alpha) {
                if (!argb565_equal((0xffff00&(p_bpp8[0]|(p_bpp8[1] << 8)|(p_bpp8[2] << 16))), 0xffff00&color)) {
                   TRACE(0,"rgba565_equal i:%d |%x|%x|",i, (0xffffff&(p_bpp8[0]|(p_bpp8[1] << 8)|p_bpp8[2] << 16)), 0xffffff&color);
                   return -1;
                }
            } else {
                color &=~(0x0000ff);
                color |= buff->alpha;
                if (!argb565_equal((p_bpp8[0]|(p_bpp8[1] << 8)|(p_bpp8[2] << 16)), 0xffffff&color)) {
                    return -1;
                }
            }
           p_bpp8 += 3;

        }
    }
    return 0;
}
