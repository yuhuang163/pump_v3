/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __PSRAM_DEBUG_H__
#define __PSRAM_DEBUG_H__

#include "hal_trace.h"

#ifdef APSRAM_DEBUG
#define APSRAM_TRACE_LINEFEED()             TRACE_OUTPUT_LINEFEED()
#define APSRAM_TRACE(a, s, ...)             TRACE_IMM(a, "APSRAM_DEBUG: " s , ##__VA_ARGS__)
#define APSRAM_TRACE_NOTS(a, s, ...)        TRACE_NOTS(TR_ATTR_IMM | a, "APSRAM_DEBUG: " s , ##__VA_ARGS__)
#define APSRAM_TRACENOCRLF_NOTS(a, s, ...)  TRACE_NOCRLF_NOTS(TR_ATTR_IMM | a, s , ##__VA_ARGS__)
#define APSRAM_TRACE_WARNING(a, s, ...)     TRACE_IMM(a, "APSRAM_WARNING: " s , ##__VA_ARGS__)
#else
#define APSRAM_TRACE_LINEFEED()
#define APSRAM_TRACE(a, s, ...)             TRACE_DUMMY(TR_ATTR_IMM | a, s , ##__VA_ARGS__)
#define APSRAM_TRACE_NOTS(a, s, ...)        TRACE_DUMMY(TR_ATTR_IMM | a, s , ##__VA_ARGS__)
#define APSRAM_TRACENOCRLF_NOTS(a, s, ...)  TRACE_DUMMY(TR_ATTR_IMM | a, s , ##__VA_ARGS__)
#endif
#define APSRAM_ASSERT(c, s, ...)            ASSERT(c, "APSRAM_ASSERT: %s" s, __func__, ##__VA_ARGS__)

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

#define T_NCK_TO_NS(clk, n)                 ((uint32_t)(((n) * 1000) / ((clk) / 1000000)))
#define T_NCK_TO_US(clk, n)                 ((n < 100) ? 1 : ((uint32_t)(((n) * 1000) / ((clk) / 1000)) + 1))
#define T_NS_TO_CYCLE_NUM(clk, ns)          ((uint32_t)(((clk) / 1000000 * (ns) + (1000 - 1)) / 1000))
#define T_US_TO_CYCLE_NUM(clk, us)          ((clk) / 1000000 * (us) + 1)
#define T_MS_TO_CYCLE_NUM(clk, ms)          ((clk) / 1000 * (ms) + 1)

#endif
