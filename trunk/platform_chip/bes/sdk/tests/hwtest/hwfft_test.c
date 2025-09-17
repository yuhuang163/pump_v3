/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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

#ifdef HWFFT_TEST

#include "hwtest.h"
#include "hal_dma.h"
#include "hal_aud.h"
#include "hal_i2s.h"
#include "hal_iomux.h"
#include "hal_cmu.h"
#include "string.h"
#include "hwfft.h"


struct hwfft_t ec_hwfft_fft;
struct hwfft_t ec_hwfft_ifft;

short fft_in_buf[64] = {0};
short fft_out_buf_hw[64] = {0};
short ifft_out_buf_hw[64] = {0};

struct hwfft_t test_hwfft_fft_64;
struct hwfft_t test_hwfft_ifft_64;

const short fft_tone_1k[] = {
    0x0000,
    0x2d5d,
    0x4026,
    0x2d5c,
    0x0000,
    0xd2a4,
    0xbfd9,
    0xd2a3,
    0x0000,
    0x2d5c,
    0x4027,
    0x2d5d,
    0x0000,
    0xd2a3,
    0xbfd9,
    0xd2a4,
    0x0001,
    0x2d5c,
    0x4026,
    0x2d5c,
    0x0000,
    0xd2a4,
    0xbfda,
    0xd2a3,
    0x0000,
    0x2d5c,
    0x4026,
    0x2d5c,
    0x0000,
    0xd2a4,
    0xbfda,
    0xd2a4,
    0xffff,
    0x2d5d,
    0x4026,
    0x2d5c,
    0x0000,
    0xd2a4,
    0xbfd9,
    0xd2a4,
    0x0000,
    0x2d5d,
    0x4026,
    0x2d5c,
    0x0000,
    0xd2a4,
    0xbfda,
    0xd2a4,
    0x0000,
    0x2d5c,
    0x4027,
    0x2d5c,
    0x0000,
    0xd2a4,
    0xbfda,
    0xd2a4,
    0x0000,
    0x2d5d,
    0x4027,
    0x2d5c,
    0x0000,
    0xd2a4,
    0xbfda,
    0xd2a4,
};

void hwfft_test(void)
{
    unsigned char i = 0;

    hwfft_create(&test_hwfft_fft_64, HWFFT_MODE_FFT|HWFFT_MODE_EQVL_FFT, HWFFT_ORDER_64SAMP);
    hwfft_create(&test_hwfft_ifft_64, HWFFT_MODE_IFFT|HWFFT_MODE_EQVL_IFFT, HWFFT_ORDER_64SAMP);

    memcpy(fft_in_buf, fft_tone_1k, sizeof(fft_tone_1k));

    hwfft_fft_process(&test_hwfft_fft_64, (char *)fft_in_buf, (char *)fft_out_buf_hw);

    TRACE(0,"fft_in_buf");
    for(i = 0;i<64;i++) {
        TRACE(1,"%x\n", fft_in_buf[i]);
    }
    TRACE(0,"\n\n");
    TRACE(0,"fft_out_buf_hw");
    for(i = 0;i<64;i++)
        TRACE(1,"%x\n", fft_out_buf_hw[i]);

    hwfft_fft_process(&test_hwfft_ifft_64, (char *)fft_out_buf_hw, (char *)ifft_out_buf_hw);

    TRACE(0,"\n\n");
    TRACE(0,"ifft_out_buf_hw");
    for(i = 0;i<64;i++)
        TRACE(1,"%x\n", ifft_out_buf_hw[i]);
}

#endif

