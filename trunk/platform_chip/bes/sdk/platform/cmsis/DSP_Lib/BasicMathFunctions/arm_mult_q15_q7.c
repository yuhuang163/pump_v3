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

#include "dsp/basic_math_functions.h"


void arm_mult_q15_q7_bias_q31(q15_t *pSrc,
                              const q7_t *wt,
                              const q31_t *bias,
                              q15_t *pDst,
                              uint32_t out_shift,
                              uint32_t blockSize)
{
    uint32_t blkCnt;

#if defined (ARM_MATH_LOOPUNROLL) && defined(ARM_MATH_DSP)
    q31_t inA1, inA2;
    q31_t out1, out2, out3, out4;

    /* Loop unrolling: Compute 4 outputs at a time */
    blkCnt = blockSize >> 2U;
    while (blkCnt > 0U) {
        inA1 = read_q15x2_ia ((q15_t **) &pSrc);
        inA2 = read_q15x2_ia ((q15_t **) &pSrc);
        out1 = *bias++;
        out2 = *bias++;
        out3 = *bias++;
        out4 = *bias++;

        out1 += (q31_t) ((q15_t) (inA1      ) * (*wt++));
        out2 += (q31_t) ((q15_t) (inA1 >> 16) * (*wt++));
        out3 += (q31_t) ((q15_t) (inA2      ) * (*wt++));
        out4 += (q31_t) ((q15_t) (inA2 >> 16) * (*wt++));

        *pDst++ = (q15_t) __SSAT(out1 >> out_shift, 16);
        *pDst++ = (q15_t) __SSAT(out2 >> out_shift, 16);
        *pDst++ = (q15_t) __SSAT(out3 >> out_shift, 16);
        *pDst++ = (q15_t) __SSAT(out4 >> out_shift, 16);

        blkCnt--;
    }

    blkCnt = blockSize % 0x4U;
#else
    blkCnt = blockSize;
#endif

    while(blkCnt > 0U) {
        q31_t out = *bias++;
        out += (q31_t) ((*pSrc++) * (*wt++));
        *pDst++ = (q15_t) __SSAT(out >> out_shift, 16);

        blkCnt--;
    }
}
