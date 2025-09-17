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
#ifndef __FUNCITON_TEST_H__
#define __FUNCITON_TEST_H__
#ifdef __cplusplus
extern "C" {
#endif

void chip_function_test(void);

void function_test_hwtimer(void);
void function_test_rtc(void);
void function_test_i2s(void);
void function_test_dma(void);
void function_test_norflash(void);
void function_test_power_reset(void);
void function_test_gpio(void);
void function_test_current(void);
void function_test_sdmmc(void);
void function_test_i2c(void);
void function_test_sam(void);

#ifdef __cplusplus
}
#endif

#endif //__FUNCITON_TEST_H__

