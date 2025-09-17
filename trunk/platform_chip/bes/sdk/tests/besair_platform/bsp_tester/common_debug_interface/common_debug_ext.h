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
#ifndef COMMON_DEBUG_EXT_H
#define COMMON_DEBUG_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common_debug.h"

#define common_CMD_HELP_INFO                                               "\n\
        help - an interface to the common debug command manual              \n\
SYNOPSIS                                                                    \n\
        common,help                                                         \n\
        common,[command],[register address],[register value]                \n\
                command options                                             \n\
                        ana_w, ana_r, w4, r4                                \n\
                        rf_w, rf_r, pmu_w, pmu_r                            \n\
                        rc_w, rc_r, pmui_w, pmui_r                          \n\
                register address options                                    \n\
                        uint32_t type in hexadecimal for digital register   \n\
                        operations (case-insensitive)                       \n\
                        uint16_t type in hexadecimal for other register     \n\
                        operations (case-insensitive)                       \n\
                register value options (for register write only)            \n\
                        uint32_t type in hexadecimal for digital register   \n\
                        operations (case-insensitive)                       \n\
                        uint16_t type in hexadecimal for other register     \n\
                        operations (case-insensitive)                       \n\
EXAMPLES                                                                    \n\
        common,help                                                         \n\
                get help manual of gpio test command                        \n\
        common,ana_w,61,0821                                                \n\
                write value 0x0821 to analog register 0x0061                \n\
        common,ana_r,61                                                     \n\
                read out value from analog register 0x0061                  \n\
        common,w4,40300088,0025B626                                         \n\
                write value 0x0325B622 to digital register 0x40300088       \n\
        common,r4,40300088                                                  \n\
               read out value from digital register 0x40300088              \n\
        common,rf_w,2B,0C00                                                 \n\
                write value 0x0C00 to rf register 0x002B                    \n\
        common,rf_r,2B                                                      \n\
                read out value from rf register 0x002B                      \n\
        common,pmu_r,C1                                                     \n\
                read out value from pmu register 0x00C1                     \n\
        common,rc_w,06,FF80                                                 \n\
                write value 0xFF80 to rc register 0x0006                    \n\
        common,rc_r,06                                                      \n\
                read out value from rc register 0x0006                    \n\n\
"

/**
 * \brief common debug function
 * \param [in]  params  parameters received as strings from uart port
 */
void common_debug(const char **params);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_DEBUG_EXT_H */