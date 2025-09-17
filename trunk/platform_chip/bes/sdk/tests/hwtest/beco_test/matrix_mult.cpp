
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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "beco.h"
#include "beco_l1.h"

#include "beco_rand64.h"
#include "naive_matrix_mult.hpp"
#include "beco_print_matrix.hpp"
#include "beco_matrix_functions.h"
#include "beco_test.h"

//#define TEST_TIME
#ifdef TEST_TIME
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "dwt.h"
#endif

// Tile size is 4x4 uint16_t's
#define MAT_M 16
#define MAT_N 16
#define MAT_K 32

uint16_t A[MAT_M*MAT_K];
uint16_t B[MAT_K*MAT_N];
uint32_t Cn[MAT_M*MAT_N];
uint32_t Cb[MAT_M*MAT_N];

/* int main(int argc, char** argv, char** env) */
int matrix_test(void)
{
#ifdef TEST_TIME
    int dwt_enable();
    dwt_enable();
#endif

    // Fill input matrix
#ifdef FILL_RANDOM
    beco_set_random_seed(1);
    beco_vec_fill_random((uint64_t*)A, sizeof(A)/sizeof(uint64_t));
    beco_vec_fill_random((uint64_t*)B, sizeof(B)/sizeof(uint64_t));
#else
    memset(A, 0, sizeof(A));
    memset(B, 0, sizeof(B));
    // 0 0...    4500 4501 4502 ...         0    0    0 ...
    // 1 0... x     0    0    0 ...    = 4500 4501 4502 ...
    // 2 0...                            9000 ...
    for(int i=0; i< 16; i++) { A[i] = i; B[i] = i+4500; }
#endif

    printf("Naive\n");

#ifdef TEST_TIME
    int start_native = hal_sys_timer_get();
    dwt_reset_cycle_cnt();
#endif

    naive_mmult_aTb(A, B, Cn, MAT_M, MAT_N, MAT_K);

#ifdef TEST_TIME
    uint32_t cyc_native = dwt_read_cycle_cnt();
    int end_native = hal_sys_timer_get();
    TRACE(0, "\nnaive_matrix_mult use %dcycle\n", (cyc_native));
    TRACE(0, "\nnaive_matrix_mult use %dus\n", TICKS_TO_US(end_native - start_native));
#endif

    print_matrix(Cn, MAT_N, MAT_M, MAT_N);

    printf("beco\n");

#ifdef TEST_TIME
    int start_beco = hal_sys_timer_get();
    dwt_reset_cycle_cnt();
#endif

    beco_mat_mult_u16xu16_u32_fastest(A, B, Cb, MAT_M, MAT_N, MAT_K);

#ifdef TEST_TIME
    uint32_t cyc_beco = dwt_read_cycle_cnt();
    int end_beco = hal_sys_timer_get();
    TRACE(0, "\nbeco_matrix_mult use %dcycle\n", cyc_beco);
    TRACE(0, "\nbeco_matrix_mult use %dus\n", TICKS_TO_US(end_beco - start_beco));
#endif

    print_matrix(Cb, MAT_N, MAT_M, MAT_N);


    return 0;
}

