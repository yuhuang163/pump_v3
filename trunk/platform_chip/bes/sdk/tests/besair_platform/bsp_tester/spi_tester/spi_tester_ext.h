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
#ifndef SPI_TESTER_EXT_H
#define SPI_TESTER_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spi_tester.h"

#define spi_CMD_HELP_INFO                                                  "\n\
        help - an interface to the spi test command manual                  \n\
SYNOPSIS                                                                    \n\
        spi,help                                                            \n\
        spi,[rate],[mode],[command]                                         \n\
                rate options (MHz)                                          \n\
                        12, 24                                              \n\
                mode options                                                \n\
                        0, 1, 2, 3                                          \n\
                        REF   mode    CPOL    CPHA                          \n\
                                0       0       0                           \n\
                                1       0       1                           \n\
                                2       1       0                           \n\
                                3       1       1                           \n\
                command options                                             \n\
                        write, read                                         \n\
EXAMPLES                                                                    \n\
        spi,help                                                            \n\
                get help manual of spi test command                         \n\
        spi,12,0,write                                                      \n\
                send messages through spi port, 12MHz speed, mode 0         \n\
        spi,24,3,read                                                       \n\
                receive messages through spi port, 24MHz speed, mode 3    \n\n\
"

/**
 * \brief spi test function
 * \param [in]  params  parameters received as strings from uart port
 */
void spi_tester(const char **params);

#ifdef __cplusplus
}
#endif

#endif /* SPI_TESTER_EXT_H */