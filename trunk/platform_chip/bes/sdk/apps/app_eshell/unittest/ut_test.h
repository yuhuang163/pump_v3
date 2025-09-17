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

#ifndef __ESHELL_UT_TEST_H__
#define __ESHELL_UT_TEST_H__

#include "eshell.h"
#include "app_eshell.h"
#include "app_utils.h"

//#define UT_DEBUG_EN

#ifdef UT_DEBUG_EN
#define UT_DBG(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define UT_DBG(...)
#endif
#define UT_LOG(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define UT_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

extern void *frame_test_buf0;
extern void *frame_test_buf1;

#define frame_test_nc_buf0 ((void *)((uint32_t)frame_test_buf0 - PSRAM_BASE + PSRAM_NC_BASE))
#define frame_test_nc_buf1 ((void *)((uint32_t)frame_test_buf1 - PSRAM_BASE + PSRAM_NC_BASE))

#endif

