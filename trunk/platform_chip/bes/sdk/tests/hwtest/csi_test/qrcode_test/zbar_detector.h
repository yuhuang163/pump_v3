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
#ifndef __ZBAR_DETECTOR__
#define __ZBAR_DETECTOR__

#include "dsp_qrcode_process.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qrcode_result_s
{
    char type[32];
    char data[512];
    uint32_t location_count;
    int32_t location_x[8];
    int32_t location_y[8];
} qrcode_result_t;

extern qrcode_result_t qrcode_result;

void zbar_init();
void qrcode_img_copy(struct qrcode_img_cfg *img);
int zbar_detect(struct qrcode_img_cfg *img);
void zbar_destory();

#ifdef __cplusplus
}
#endif

#endif
