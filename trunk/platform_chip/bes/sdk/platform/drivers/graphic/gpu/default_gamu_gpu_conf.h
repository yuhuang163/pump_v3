
/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#ifndef _GPU_DEFAULT_CFG_H
#define _GPU_DEFAULT_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "default_gamu_graphic_conf.h"

#define    GPU_FB_WIDTH    464
#define    GPU_FB_HEIGHT   464

/*
 * CONFIG_LCDC_FB_BPP is defined at default_gamu_graphic_conf.h
*/
#if (CONFIG_LCDC_FB_BPP == 24)
    #define    GPU_FB_FORMAT   VG_LITE_RGB888
    #define    GPU_FB_BPP      24
#elif (CONFIG_LCDC_FB_BPP == 16)
    #define    GPU_FB_FORMAT   VG_LITE_RGB565
    #define    GPU_FB_BPP      16
#else
    #define    GPU_FB_FORMAT   VG_LITE_RGBA8888
    #define    GPU_FB_BPP      32
#endif

/*
 *  GPU_CONTIGUOUS_SIZE, the heep size of gpu driver,
 *  gpu driver use it to allocat command,tessilation and image buffers.
 *  minimal size can be set to 125 kB, if not allocate any image buffer by
 *  gpu driver
*/
#define    GPU_CONTIGUOUS_SIZE   (1024*1024*4)

#ifdef __cplusplus
}
#endif

#endif //_GPU_DEFAULT_CFG_H
