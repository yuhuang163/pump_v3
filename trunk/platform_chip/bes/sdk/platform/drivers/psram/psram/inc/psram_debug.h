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
#ifndef __PSRAM_DEBUG_H
#define __PSRAM_DEBUG_H

#include "hal_trace.h"

#ifdef PSRAM_DEBUG
#define PSRAM_TRACE_LINEFEED()              TRACE_OUTPUT_LINEFEED()
#define PSRAM_TRACE(a, s, ...)              TRACE_IMM(a, "PSRAM_DEBUG: " s , ##__VA_ARGS__)
#define PSRAM_TRACE_NOTS(a, s, ...)         TRACE_NOTS(TR_ATTR_IMM | a, "PSRAM_DEBUG: " s , ##__VA_ARGS__)
#define PSRAM_TRACENOCRLF_NOTS(a, s, ...)   TRACE_NOCRLF_NOTS(TR_ATTR_IMM | a, s , ##__VA_ARGS__)
#define PSRAM_TRACE_WARNING(a, s, ...)      TRACE_IMM(a, "PSRAM_WARNING: " s , ##__VA_ARGS__)
#else
#define PSRAM_TRACE_LINEFEED()
#define PSRAM_TRACE(a, s, ...)              TRACE_DUMMY(TR_ATTR_IMM | a, s , ##__VA_ARGS__)
#define PSRAM_TRACE_NOTS(a, s, ...)         TRACE_DUMMY(TR_ATTR_IMM | a, s , ##__VA_ARGS__)
#define PSRAM_TRACENOCRLF_NOTS(a, s, ...)   TRACE_DUMMY(TR_ATTR_IMM | a, s , ##__VA_ARGS__)
#endif

#define PSRAM_ASSERT(c, s, ...)             ASSERT(c, "PSRAM_ASSERT: %s" s, __func__, ##__VA_ARGS__)

#endif /*__PSRAM_DEBUG_H*/
