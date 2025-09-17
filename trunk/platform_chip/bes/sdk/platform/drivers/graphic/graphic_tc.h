
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

#ifndef __GRAPHIC_TC_H_
#define __GRAPHIC_TC_H_

#ifdef GRAPHICDRV_TIME_STATIS

#include "hal_timer.h"
#include "hal_trace.h"

#define _TEST_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)

#define TC_INIT  volatile uint32_t time_s; \
                 volatile uint32_t time_e;
#define TC_START time_s = hal_fast_sys_timer_get();
#define TC_END   time_e = hal_fast_sys_timer_get();

#define _TSTR(s) #s

#define TC_REP(s) do{ \
                      _TEST_REP(_TSTR(s)":%d us", FAST_TICKS_TO_US(time_e - time_s)); \
                    }while(0)

#else

#define TC_INIT
#define TC_START
#define TC_END
#define TC_REP(s)

#endif

#endif /*__GRAPHIC_TC_H_*/
