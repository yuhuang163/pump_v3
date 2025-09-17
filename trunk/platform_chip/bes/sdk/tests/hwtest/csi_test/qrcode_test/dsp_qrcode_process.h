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
#ifndef __DSP_QRCODE_PROCESS_H__
#define __DSP_QRCODE_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

struct qrcode_img_cfg {
    uint32_t addr;
    uint16_t height;
    uint16_t width;
    uint16_t bpp;
    uint16_t fmt;
};

int dsp_qrcode_process_init(void);

void dsp_qrcode_rpc_rx_handler(void *data, int size);

void qrcode_get_img(uint32_t addr, uint16_t height, uint16_t width, uint16_t bpp, uint16_t fmt);

#ifdef __cplusplus
}
#endif

#endif
