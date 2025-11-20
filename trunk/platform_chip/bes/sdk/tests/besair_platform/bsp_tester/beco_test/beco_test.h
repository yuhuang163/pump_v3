/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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


#ifndef __BECO_TEST_H__
#define __BECO_TEST_H__

#include "beco_nnfunctions.h"
#include "beco_nnsupportfunctions.h"
#include "arm_nnfunctions.h"

BECO_C_DECLARATIONS_START

void beco_api_test(void);

int matrix_test(void);

void beco_fully_connected_s8_test(void);

void beco_fully_connected_s16_test(void);

void beco_convolve_s8_test(void);

void beco_convolve_s16_test(void);

void beco_convolve_1x1_s8_test(void);

void beco_convolve_1x1_s16_test(void);

void beco_depthwise_conv_nx3_s8_test(void);
void beco_transpose_depthwise_conv_s8_test(void);

void beco_depthwise_conv_nx3_s16_test(void);
void beco_transpose_depthwise_conv_s16_test(void);

void beco_avgpool_s8_test(void);

void beco_avgpool_s16_test(void);

void beco_max_pool_s8_test(void);

void beco_max_pool_s16_test(void);

void beco_gru_s16_pt_test(void);
void beco_gru_s16_pc_test(void);
void beco_bi_gru_s16_test(void);

void beco_lstm_s16_pt_test(void);
void beco_lstm_s16_pc_test(void);
void beco_bi_lstm_s16_test(void);

BECO_C_DECLARATIONS_END
#endif /*__BECO_TEST_H__*/
