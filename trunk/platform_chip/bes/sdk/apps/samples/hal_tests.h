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
#ifndef __HAL_TESTS_H__
#define __HAL_TESTS_H__

#include "plat_types.h"

void gpio_wakeup_test(char* cmdParam, uint32_t cmdParam_len);

void uart1_demo_test(char* cmdParam, uint32_t cmdParam_len);

void gpadc_test(char* cmdParam, uint32_t cmdParam_len);

void hw_timer_test(char* cmdParam, uint32_t cmdParam_len);

#endif /* #ifndef __HAL_TESTS_H__ */
