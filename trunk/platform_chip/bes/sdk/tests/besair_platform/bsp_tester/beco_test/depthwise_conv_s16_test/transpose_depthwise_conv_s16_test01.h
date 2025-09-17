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

#define TRANSPOSE_DEPTH_S16_TEST01_INPUT_CH 1
#define TRANSPOSE_DEPTH_S16_TEST01_INPUT_H 1
#define TRANSPOSE_DEPTH_S16_TEST01_INPUT_W 15
#define TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_CH 1
#define TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_H 1
#define TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_W 31
#define TRANSPOSE_DEPTH_S16_TEST01_PADDING_X 0
#define TRANSPOSE_DEPTH_S16_TEST01_PADDING_Y 1
#define TRANSPOSE_DEPTH_S16_TEST01_STRIDE_X 2
#define TRANSPOSE_DEPTH_S16_TEST01_STRIDE_Y 1
#define TRANSPOSE_DEPTH_S16_TEST01_DILATION_X 1
#define TRANSPOSE_DEPTH_S16_TEST01_DILATION_Y 1
#define TRANSPOSE_DEPTH_S16_TEST01_KERNEL_X 3
#define TRANSPOSE_DEPTH_S16_TEST01_KERNEL_Y 3

#define TRANSPOSE_DEPTH_S16_TEST01_INPUT_OFFSET 0
#define TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_OFFSET 0

const int8_t transpose_depth_s16_test01_beco_weight[] = {
    41, -12, -73, -118, -127, 21, -117, -112, 78,
};

int32_t transpose_depth_s16_test01_multiplier[] = {
    1192979767
};

int32_t transpose_depth_s16_test01_shift[] = {
    -6
};

const int32_t transpose_depth_s16_test01_beco_bias[] = {
    1157079
};

const int16_t transpose_depth_s16_test01_beco_input[] = {
    20764, -8278, 22347, 4792, 2379, 2038, -20613, -18044, 6062, 19379, -16794, 7544, 9353, 32767, -9602,
};

/*
reference output:
13781, -12846, -12648, 19169, 22517, -14591, -11912, 4761, 5578, 7421, 7981, 7797, 4253, 32767, 27844, 29935, 29559, 3361, 7341, -11319, -12766, 28556, 28549, 1727, 4023, -267, 6391, -26078, -25142, 20628, 19847,
*/
