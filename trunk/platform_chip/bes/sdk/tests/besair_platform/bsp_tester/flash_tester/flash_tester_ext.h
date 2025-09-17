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
#ifndef FLASH_TESTER_EXT_H
#define FLASH_TESTER_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "flash_tester.h"

#define flash_CMD_HELP_INFO                                                "\n\
        help - an interface to the flash test command manual                \n\
SYNOPSIS                                                                    \n\
        flash,help                                                          \n\
        flash,[flash port],[command]                                        \n\
                command options                                             \n\
                        erase, write, read                                  \n\
EXAMPLES                                                                    \n\
        flash,help                                                          \n\
                get help manual of flash 0 test command                     \n\
        flash,0,erase                                                       \n\
                erase 4K bytes in nor flash 0                               \n\
        flash,0,write                                                       \n\
                write 4K bytes in nor flash 0                               \n\
        flash,0,read                                                        \n\
                read out 100 bytes from nor flash 0                       \n\n\
"

/**
 * \brief flash test function
 * \param [in]  params  parameters received as strings from uart port
 */
void flash_tester(const char **params);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_TESTER_EXT_H */