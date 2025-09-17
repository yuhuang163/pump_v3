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
#ifdef CHIP_FUNCTION_TEST_CURRENT
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_trace.h"
 #include "string.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "pmu.h"

#include "string.h"

#define MAT_M 16
#define MAT_N 16
#define MAT_K 32

static uint16_t A[MAT_M*MAT_K];
static uint16_t B[MAT_K*MAT_N];
static uint32_t Cn[MAT_M*MAT_N];
//static uint32_t Cb[MAT_M*MAT_N];

static void naive_mmult_aTb(const uint16_t *A, const uint16_t *B, uint32_t *C, int M, int N, int K)
{
    int i,j,p;
    uint32_t dot_p;

    for (i = 0; i < M; i++) {
      for (j = 0; j < N; j++) {
        // Calculate the dot product between A.column and B.column:
        dot_p = 0;
        for (p = 0; p < K; p++) {
           dot_p += A[i+p*M] * B[p*N+j]; // A[M][K] * B[K][N]
        }
        C[i*N+j] = dot_p;
      }
    }
}

POSSIBLY_UNUSED static void _matrix_test(void)
{
    memset(A, 0, sizeof(A));
    memset(B, 0, sizeof(B));
    // 0 0...    4500 4501 4502 ...         0    0    0 ...
    // 1 0... x     0    0    0 ...    = 4500 4501 4502 ...
    // 2 0...                            9000 ...
    for(int i=0; i< 16; i++) { A[i] = i; B[i] = i+4500; }

    for (int k = 0; k < 1000; k++) {
        naive_mmult_aTb(A, B, Cn, MAT_M, MAT_N, MAT_K);
    }
}

#ifdef WFI_TEST
static void function_test_wfi(void)
{
    while (1) {
        __WFI();
    }
}
#endif

#ifdef WHILE1_TEST
static void function_test_while1(void)
{
    while (1);
}
#endif

void function_test_current(void)
{
    enum HAL_CMU_FREQ_T sys_freq;
    TRACE(1, "%s: begin", __func__);

#ifdef CURRENT_TEST_SYSFREQ
    sys_freq = CURRENT_TEST_SYSFREQ;
#else
    sys_freq = HAL_CMU_FREQ_26M;
#endif

    TR_INFO(TR_MOD(MAIN), "function test current sysfreq=%d", sys_freq);
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
    TR_INFO(TR_MOD(MAIN), "function test current CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

#ifdef WFI_TEST
    function_test_wfi();
#elif defined(WHILE1_TEST)
    function_test_while1();
#else
    while(1)
    {
        _matrix_test();
    }
#endif
}
#endif
