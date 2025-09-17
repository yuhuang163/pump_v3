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
#ifndef WDT_TESTER_EXT_H
#define WDT_TESTER_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wdt_tester.h"

#define wdt_CMD_HELP_INFO                                                  "\n\
        help - an interface to the wdt test command manual                  \n\
SYNOPSIS                                                                    \n\
        wdt,help                                                            \n\
        wdt,[feed period],[timeout time]                                    \n\
                feed period options                                         \n\
                        unsigned int type range in second                   \n\
                timeout time options                                        \n\
                        unsigned int type range in second                   \n\
EXAMPLES                                                                    \n\
        wdt,help                                                            \n\
                get help manual of wdt test command                         \n\
        wdt,1,10                                                            \n\
                feed watchdog timer every 1 sec, 10 secs for timeout        \n\
        wdt,10,1                                                            \n\
                feed watchdog timer every 10 secs, 1 sec for timeout      \n\n\
"

/**
 * \brief wdt test function
 * \param [in]  params  parameters received as strings from uart port
 */
void wdt_tester(const char **params);

#ifdef __cplusplus
}
#endif

#endif /* WDT_TESTER_EXT_H */