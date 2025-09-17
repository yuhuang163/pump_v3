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
#ifndef __CRASH_ANALYSIS_TESTS_H__
#define __CRASH_ANALYSIS_TESTS_H__

#include "plat_types.h"

void data_access_error_test(char* cmdParam, uint32_t cmdParam_len);

void instruction_bus_error_test(char *cmdParam, uint32_t cmdParam_len);

void data_bus_error_test(char *cmdParam, uint32_t cmdParam_len);

void usage_fault_test(char *cmdParam, uint32_t cmdParam_len);

void hard_fault_test(char *cmdParam, uint32_t cmdParam_len);

void dead_lock_test(char *cmdParam, uint32_t cmdParam_len);

void stack_overflow_test(char *cmdParam, uint32_t cmdParam_len);

#endif /* #ifndef __CRASH_ANALYSIS_TESTS_H__ */
