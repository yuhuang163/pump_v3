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
#ifdef CMSIS_TEST
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"

#include "arm_math.h"
#include "cmsis.h"
#include "dwt.h"

#include "dsp/filtering_functions_f16.h"
#include "dsp/basic_math_functions_f16.h"

#undef MIN
#undef MAX
#include "arm_nnsupportfunctions.h"

#define TEST_LENGTH_SAMPLES 1024
#define TEST_FIR_BLOCK_SAMPLES 512
#define TEST_FIR_COEF_LEN 512

static float32_t testInput_f32[TEST_LENGTH_SAMPLES+TEST_FIR_BLOCK_SAMPLES];
static float32_t testOut_f32[TEST_LENGTH_SAMPLES+TEST_FIR_BLOCK_SAMPLES];
static float32_t teststate_f32[TEST_FIR_BLOCK_SAMPLES*3+TEST_FIR_COEF_LEN];

#if defined(ARM_MATH_MVE_FLOAT16)
static float16_t testInput_f16[TEST_LENGTH_SAMPLES+TEST_FIR_BLOCK_SAMPLES];
static float16_t testOut_f16[TEST_LENGTH_SAMPLES+TEST_FIR_BLOCK_SAMPLES];
static float16_t teststate_f16[TEST_FIR_BLOCK_SAMPLES*4+TEST_FIR_COEF_LEN];
#endif

static uint16_t fft_Size = 0;
static arm_cfft_instance_f32 varInstCfftF32;
static arm_rfft_fast_instance_f32 varInstRfftF32;

static q31_t testInput_q31[TEST_LENGTH_SAMPLES+TEST_FIR_BLOCK_SAMPLES];
static q31_t testOut_q31[TEST_LENGTH_SAMPLES+TEST_FIR_BLOCK_SAMPLES];
static q31_t teststate_q31[TEST_FIR_BLOCK_SAMPLES*3+TEST_FIR_COEF_LEN];

static q7_t testInput_q7[TEST_LENGTH_SAMPLES+TEST_FIR_BLOCK_SAMPLES];
static q7_t testOut_q7[TEST_LENGTH_SAMPLES+TEST_FIR_BLOCK_SAMPLES];
static q7_t teststate_q7[TEST_FIR_BLOCK_SAMPLES*4+TEST_FIR_COEF_LEN];

static arm_cfft_instance_q31 varInstCfftQ31;
static arm_rfft_instance_q31 varInstRfftQ31;

static arm_fir_instance_f32 varInstfirf32;
static arm_fir_instance_q31 varInstfirq31;
#if defined(ARM_MATH_MVE_FLOAT16)
static arm_fir_instance_f16 varInstfirf16;
#endif
static arm_fir_instance_q7 varInstfirq7;

SRAM_TEXT_LOC
static void test_dsp_cfft_f32(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    TRACE_IMM(0, "%s", __func__);

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_f32[2*i] = (1.0f + i%2);
        testInput_f32[2*i+1] = 0;
    }
    uint32_t cyc_cnt = -1u;

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    dwt_enable();

    arm_status status;
    TRACE_IMM(0, "%s fft_Size:%d", __func__,fft_Size);

    status = ARM_MATH_SUCCESS;
    status = arm_cfft_init_f32(&varInstCfftF32,fft_Size);

    TRACE_IMM(0, "%s status:%d", __func__,status);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();
        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_cfft_f32(&varInstCfftF32, testInput_f32, 0, 1);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);

        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}

SRAM_TEXT_LOC
static void test_dsp_cfft_q31(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_q31[2*i] = i;
        testInput_q31[2*i+1] = i+1;
    }

    arm_status status;
    TRACE_IMM(0, "%s fft_Size:%d", __func__,fft_Size);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    status = ARM_MATH_SUCCESS;
    status = arm_cfft_init_q31(&varInstCfftQ31,fft_Size);

    TRACE_IMM(0, "%s status:%d", __func__,status);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();


        /* Process the data through the CFFT/CIFFT module */
        arm_cfft_q31(&varInstCfftQ31, testInput_q31, 0, 1);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}

SRAM_TEXT_LOC
static void test_dsp_rfft_f32(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    TRACE_IMM(0, "%s", __func__);

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_f32[2*i] = (1.0f + i%2);
        testInput_f32[2*i+1] = 0;
    }
    uint32_t cyc_cnt = -1u;

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    dwt_enable();

    arm_status status;
    TRACE_IMM(0, "%s fft_Size:%d", __func__,fft_Size);

    status = ARM_MATH_SUCCESS;
    status = arm_rfft_fast_init_f32(&varInstRfftF32,fft_Size);

    TRACE_IMM(0, "%s status:%d", __func__,status);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();
        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_rfft_fast_f32(&varInstRfftF32, testInput_f32, testOut_f32, 0);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);

        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}

SRAM_TEXT_LOC
static void test_dsp_rfft_q31(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_q31[2*i] = i;
        testInput_q31[2*i+1] = i+1;
    }

    arm_status status;
    TRACE_IMM(0, "%s fft_Size:%d", __func__,fft_Size);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    status = ARM_MATH_SUCCESS;
    status = arm_rfft_init_q31(&varInstRfftQ31,fft_Size,0,1);

    TRACE_IMM(0, "%s status:%d", __func__,status);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_rfft_q31(&varInstRfftQ31, testInput_q31, testOut_q31);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}

SRAM_TEXT_LOC
static void test_dsp_fir_f32(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_f32[2*i] = i;
        testInput_f32[2*i+1] = i+1;
    }

    TRACE_IMM(0, "%s TEST_FIR_COEF_LEN:%d", __func__,TEST_FIR_COEF_LEN);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    arm_fir_init_f32(&varInstfirf32,TEST_FIR_COEF_LEN,testInput_f32,teststate_f32,TEST_FIR_BLOCK_SAMPLES);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_fir_f32(&varInstfirf32, testInput_f32, testOut_f32,TEST_FIR_BLOCK_SAMPLES);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}

SRAM_TEXT_LOC
static void test_dsp_fir_q31(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_q31[2*i] = i;
        testInput_q31[2*i+1] = i+1;
    }

    TRACE_IMM(0, "%s TEST_FIR_COEF_LEN:%d", __func__,TEST_FIR_COEF_LEN);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    arm_fir_init_q31(&varInstfirq31,TEST_FIR_COEF_LEN,testInput_q31,teststate_q31,TEST_FIR_BLOCK_SAMPLES);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_fir_q31(&varInstfirq31, testInput_q31, testOut_q31,TEST_FIR_BLOCK_SAMPLES);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}


#if defined(ARM_MATH_MVE_FLOAT16)
SRAM_TEXT_LOC
static void test_dsp_fir_f16(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_f16[2*i] = i;
        testInput_f16[2*i+1] = i+1;
    }

    TRACE_IMM(0, "%s TEST_FIR_COEF_LEN:%d", __func__,TEST_FIR_COEF_LEN);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    arm_fir_init_f16(&varInstfirf16,TEST_FIR_COEF_LEN,testInput_f16,teststate_f16,TEST_FIR_BLOCK_SAMPLES);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_fir_f16(&varInstfirf16, testInput_f16, testOut_f16,TEST_FIR_BLOCK_SAMPLES);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}
#endif

SRAM_TEXT_LOC
static void test_dsp_fir_q7(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_q7[2*i] = i;
        testInput_q7[2*i+1] = i+1;
    }

    TRACE_IMM(0, "%s TEST_FIR_COEF_LEN:%d", __func__,TEST_FIR_COEF_LEN);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    arm_fir_init_q7(&varInstfirq7,TEST_FIR_COEF_LEN,testInput_q7,teststate_q7,TEST_FIR_BLOCK_SAMPLES);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_fir_q7(&varInstfirq7, testInput_q7, testOut_q7,TEST_FIR_BLOCK_SAMPLES);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}

#if defined(ARM_MATH_MVE_FLOAT16)
SRAM_TEXT_LOC
static void test_dsp_mult_f16(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_f16[2*i] = i;
        testInput_f16[2*i+1] = i+1;
    }

    TRACE_IMM(0, "%s mult：:%d", __func__,TEST_FIR_COEF_LEN);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_mult_f16(testInput_f16, testOut_f16,testOut_f16,TEST_FIR_BLOCK_SAMPLES);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}
#else
static void test_dsp_mult_f32(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_f32[2*i] = i;
        testInput_f32[2*i+1] = i+1;
    }

    TRACE_IMM(0, "%s mult：:%d", __func__,TEST_FIR_COEF_LEN);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_mult_f32(testInput_f32, testOut_f32,testOut_f32,TEST_FIR_BLOCK_SAMPLES);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}
#endif

SRAM_TEXT_LOC
static void test_dsp_mult_q7(void)
{
    uint32_t starttime, endtime;
    uint32_t te;

    uint32_t cyc_cnt = -1u;
    TRACE_IMM(0, "%s", __func__);

    dwt_enable();

    fft_Size = TEST_LENGTH_SAMPLES/2;
    for (int i = 0; i < fft_Size; i++) {
        testInput_q7[2*i] = i;
        testInput_q7[2*i+1] = i+1;
    }

    TRACE_IMM(0, "%s TEST_FIR_COEF_LEN:%d", __func__,TEST_FIR_COEF_LEN);

    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    for(int i=0;i<10;i++) {
        uint32_t lock=int_lock();

        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();

        /* Process the data through the CFFT/CIFFT module */
        arm_mult_q7(testInput_q7, testOut_q7,testOut_q7,TEST_FIR_BLOCK_SAMPLES);

        cyc_cnt = dwt_read_cycle_cnt();

        endtime = hal_fast_sys_timer_get();
        int_unlock(lock);

        te = FAST_TICKS_TO_US(endtime - starttime);
        TRACE_IMM(0, "%s,i=%d: cyc_cnt=%d；time=%d us", __func__,i,cyc_cnt,te);
        TRACE_FLUSH();
        hal_sys_timer_delay(16);
    }
}

SRAM_TEXT_LOC
void cmsis_test(void)
{
    TRACE_IMM(0, "%s", __func__);

    test_dsp_cfft_f32();
    test_dsp_cfft_q31();
    test_dsp_rfft_f32();
    test_dsp_rfft_q31();
    test_dsp_fir_f32();
    test_dsp_fir_q31();
#if defined(ARM_MATH_MVE_FLOAT16)
    test_dsp_fir_f16();
#endif
    test_dsp_fir_q7();
#if defined(ARM_MATH_MVE_FLOAT16)
    test_dsp_mult_f16();
#else
    test_dsp_mult_f32();
#endif
    test_dsp_mult_q7();
}
#endif
