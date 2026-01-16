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

#ifndef __VG_LITE_CONFIG_H__
#define __VG_LITE_CONFIG_H__

#if defined (CHIP_BEST1502X)
#include "options_413.h"
#elif defined (CHIP_BEST1502P)
#include "options_424.h"
#elif defined (CHIP_BEST1503)
#include "options_424.h"
#undef  gcFEATURE_VG_MATH_PRECISION_FIX
#define gcFEATURE_VG_MATH_PRECISION_FIX 0
#elif defined (CHIP_BEST1306P)
#include "options_424.h"
#elif defined (CHIP_BEST1600)
#include "options_40c.h"
#elif defined (CHIP_BEST1605)
#include "options_430.h"
#elif defined (CHIP_BEST1700)
#include "options_420.h"
#elif defined (CHIP_BEST1701)
#include "options_430.h"
#elif defined (CHIP_BEST3601P)
#include "options_430.h"
#elif defined (CHIP_BEST2009)
#include "options_40c.h"
#elif defined (CHIP_BEST3601)
#include "options_420.h"
#elif defined (CHIP_BEST1503P)
#include "options_433.h"
#else
#error "vglite config file is not found!"
#endif

#endif
