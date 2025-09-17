/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#pragma once
#include <stdint.h>

#define TRANSPOSE_DEPTH_S8_TEST01_INPUT_CH 1
#define TRANSPOSE_DEPTH_S8_TEST01_INPUT_H 1
#define TRANSPOSE_DEPTH_S8_TEST01_INPUT_W 15
#define TRANSPOSE_DEPTH_S8_TEST01_OUTPUT_CH 1
#define TRANSPOSE_DEPTH_S8_TEST01_OUTPUT_H 1
#define TRANSPOSE_DEPTH_S8_TEST01_OUTPUT_W 31
#define TRANSPOSE_DEPTH_S8_TEST01_PADDING_X 0
#define TRANSPOSE_DEPTH_S8_TEST01_PADDING_Y 1
#define TRANSPOSE_DEPTH_S8_TEST01_STRIDE_X 2
#define TRANSPOSE_DEPTH_S8_TEST01_STRIDE_Y 1
#define TRANSPOSE_DEPTH_S8_TEST01_DILATION_X 1
#define TRANSPOSE_DEPTH_S8_TEST01_DILATION_Y 1
#define TRANSPOSE_DEPTH_S8_TEST01_KERNEL_X 3
#define TRANSPOSE_DEPTH_S8_TEST01_KERNEL_Y 3

#define TRANSPOSE_DEPTH_S8_TEST01_INPUT_OFFSET -7
#define TRANSPOSE_DEPTH_S8_TEST01_OUTPUT_OFFSET -31

const int8_t transpose_depth_s8_test01_beco_weight[] = {
    44, -125, -65, -127, -38, -102, 21, -127, -73,
};

int32_t transpose_depth_s8_test01_multiplier[] = {
    1911433516
};

int32_t transpose_depth_s8_test01_shift[] = {
    -7
};

const int32_t transpose_depth_s8_test01_beco_bias[] = {
    9599
};

const int8_t transpose_depth_s8_test01_beco_input[] = {
    33, -2, -88, -45, 52, -10, 26, -128, 127, 47, -25, -89, 25, -8, -1,
};

/*
reference output:
-11, 0, -10, 9, 81, 32, 127, 21, 21, -5, -21, 11, 8, 2, 85, 43, 41, -25, -127, -4, -6, 15, 103, 32, 79, 2, 2, 11, 26, 9, 14,
*/
