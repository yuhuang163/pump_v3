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

#include "beco_nnsupportfunctions.h"

void beco_fully_connected_set_bias(const beco_vec64_in_t *bias)
{
    beco_write_reg(BECO_REG0, *bias++);
    beco_write_reg(BECO_REG1, *bias++);
    beco_write_reg(BECO_REG2, *bias++);
    beco_write_reg(BECO_REG3, *bias++);
    beco_set_acc_bias(BECO_ACC0_0, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_4, BECO_REG1, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_8, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_12, BECO_REG3, BECO_REG7);

    beco_write_reg(BECO_REG0, *bias++);
    beco_write_reg(BECO_REG1, *bias++);
    beco_write_reg(BECO_REG2, *bias++);
    beco_write_reg(BECO_REG3, *bias++);
    beco_set_acc_bias(BECO_ACC1_0, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC1_4, BECO_REG1, BECO_REG7);
    beco_set_acc_bias(BECO_ACC1_8, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC1_12, BECO_REG3, BECO_REG7);

    beco_write_reg(BECO_REG0, *bias++);
    beco_write_reg(BECO_REG1, *bias++);
    beco_write_reg(BECO_REG2, *bias++);
    beco_write_reg(BECO_REG3, *bias++);
    beco_set_acc_bias(BECO_ACC2_0, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC2_4, BECO_REG1, BECO_REG7);
    beco_set_acc_bias(BECO_ACC2_8, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC2_12, BECO_REG3, BECO_REG7);

    beco_write_reg(BECO_REG0, *bias++);
    beco_write_reg(BECO_REG1, *bias++);
    beco_write_reg(BECO_REG2, *bias++);
    beco_write_reg(BECO_REG3, *bias++);
    beco_set_acc_bias(BECO_ACC3_0, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC3_4, BECO_REG1, BECO_REG7);
    beco_set_acc_bias(BECO_ACC3_8, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC3_12, BECO_REG3, BECO_REG7);
}

void beco_convolve_q7_set_bias(const beco_vec64_in_t *beco_bias)
{
    beco_write_reg(BECO_REG0, *beco_bias++);
    beco_write_reg(BECO_REG1, *beco_bias++);
    beco_write_reg(BECO_REG2, *beco_bias++);
    beco_write_reg(BECO_REG3, *beco_bias++);

    beco_set_acc_bias(BECO_ACC0_0, BECO_REG0, BECO_REG0);
    beco_set_acc_bias(BECO_ACC0_4, BECO_REG0, BECO_REG0);
    beco_set_acc_bias(BECO_ACC0_8, BECO_REG1, BECO_REG1);
    beco_set_acc_bias(BECO_ACC0_12, BECO_REG1, BECO_REG1);

    beco_set_acc_bias(BECO_ACC1_0, BECO_REG2, BECO_REG2);
    beco_set_acc_bias(BECO_ACC1_4, BECO_REG2, BECO_REG2);
    beco_set_acc_bias(BECO_ACC1_8, BECO_REG3, BECO_REG3);
    beco_set_acc_bias(BECO_ACC1_12, BECO_REG3, BECO_REG3);

    beco_set_acc_bias(BECO_ACC2_0, BECO_REG0, BECO_REG0);
    beco_set_acc_bias(BECO_ACC2_4, BECO_REG0, BECO_REG0);
    beco_set_acc_bias(BECO_ACC2_8, BECO_REG1, BECO_REG1);
    beco_set_acc_bias(BECO_ACC2_12, BECO_REG1, BECO_REG1);

    beco_set_acc_bias(BECO_ACC3_0, BECO_REG2, BECO_REG2);
    beco_set_acc_bias(BECO_ACC3_4, BECO_REG2, BECO_REG2);
    beco_set_acc_bias(BECO_ACC3_8, BECO_REG3, BECO_REG3);
    beco_set_acc_bias(BECO_ACC3_12, BECO_REG3, BECO_REG3);
}

void beco_convolve_q15_set_bias(const beco_vec64_in_t *beco_bias)
{
    beco_write_reg(BECO_REG0, *beco_bias++);
    beco_write_reg(BECO_REG1, *beco_bias++);
    beco_write_reg(BECO_REG2, *beco_bias++);
    beco_write_reg(BECO_REG3, *beco_bias++);

    beco_set_acc_bias(BECO_ACC0_0, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_4, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_8, BECO_REG1, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_12, BECO_REG1, BECO_REG7);

    beco_set_acc_bias(BECO_ACC1_0, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC1_4, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC1_8, BECO_REG3, BECO_REG7);
    beco_set_acc_bias(BECO_ACC1_12, BECO_REG3, BECO_REG7);

    beco_set_acc_bias(BECO_ACC2_0, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC2_4, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC2_8, BECO_REG1, BECO_REG7);
    beco_set_acc_bias(BECO_ACC2_12, BECO_REG1, BECO_REG7);

    beco_set_acc_bias(BECO_ACC3_0, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC3_4, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC3_8, BECO_REG3, BECO_REG7);
    beco_set_acc_bias(BECO_ACC3_12, BECO_REG3, BECO_REG7);
}
