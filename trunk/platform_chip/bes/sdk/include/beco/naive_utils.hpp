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

#ifndef _NAIVE_UTILS_H
#define _NAIVE_UTILS_H

#include "stdint.h"
#include "string.h"

// weight transpose for fully connected
int getNext(int i, int m, int n)
{
    return (i%n)*m + i / n;
}

int getPre(int i, int m, int n)
{
    return (i%m)*n + i / m;
}

template <typename T>
void movedata(T *mtx, int i, int m, int n)
{
    int temp = mtx[i];
    int cur = i;
    int pre = getPre(cur, m, n);
    while (pre != i) {
        mtx[cur] = mtx[pre];
        cur = pre;
        pre = getPre(cur, m, n);
    }
    mtx[cur] = temp;
}


// input: CHW -> HWC
// kernel: Cin,HW,Cout -> Cout,HW,Cin
template <typename T_im>
static void CMSIS_Input_Transform(T_im *im, int8_t *wt, const int ICH,
                                  const int IW,const int IH,
                                  const int OCH, const int KX, const int KY)
{
    T_im temp_im[ICH*IW*IH];
    int8_t temp_wt[OCH*KX*KY*ICH];
    // transform input
    T_im *temp_im2 = temp_im;
    for(int i = 0; i < IH; i++) {
        for(int j = 0; j < IW; j++) {
            for(int k = 0; k < ICH; k++) {
                *temp_im2++ = im[k*IH*IW + i*IW + j];
            }
        }
    }

    memcpy(im, temp_im, sizeof(*im)*ICH*IW*IH);
    // transform kernel
    int8_t *temp_wt2 = temp_wt;
    for(int i = 0; i < OCH; i++){
        for(int j = 0; j < KY; j++){
            for(int k = 0; k < KX; k++){
                for(int m = 0; m < ICH; m++){
                    *temp_wt2++ = wt[m*KY*KX*OCH + j*KX*OCH + k*OCH + i];
                }
            }
        }
    }
    memcpy(wt, temp_wt, sizeof(*wt)*OCH*KY*KX*ICH);
}

//output: HWC -> CHW
template <typename T_im>
static void CMSIS_Output_Transform(T_im *cmsis_out, const int OCH, const int OW, const int OH)
{
    T_im cmsis_temp_out[OCH*OW*OH];
    T_im *cmsis_temp_out2 = cmsis_temp_out;
    for(int i = 0; i < OCH; i++) {
        for(int j = 0; j < OH; j++) {
            for(int k = 0; k < OW; k++) {
                *cmsis_temp_out2++ = cmsis_out[j*OW*OCH + k*OCH + i];
            }
        }
    }
    memcpy(cmsis_out, cmsis_temp_out, sizeof(*cmsis_out)*OCH*OW*OH);
}


template <typename Ti, typename To>
void chw2hwc(Ti *in, To *out, int dim_x, int dim_y, int dim_ch)
{
    int i = 0;
    for (i = 0; i < dim_x*dim_y*dim_ch; i++)
        out[i] = in[(int)i % dim_ch * (dim_x*dim_y) + (int)i / dim_ch];
}

template <typename Ti, typename To>
void hwc2chw(Ti *in, To *out, int dim_x, int dim_y, int dim_ch)
{
    for (int i = 0; i < dim_x*dim_y*dim_ch; i++)
        out[i] = in[(int)i % (dim_x*dim_y) * dim_ch +(int)i / (dim_x*dim_y)];
}


#endif
