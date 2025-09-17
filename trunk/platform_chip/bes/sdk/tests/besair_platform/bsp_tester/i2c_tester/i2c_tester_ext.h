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
#ifndef I2C_TESTER_EXT_H
#define I2C_TESTER_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_tester.h"

#define i2c_CMD_HELP_INFO                                                  "\n\
        help - an interface to the i2c test command manual                  \n\
SYNOPSIS                                                                    \n\
        i2c,help                                                            \n\
        i2c,[i2c port],[rate],[mode],[command],[device address],            \n\
            [register],[register length],[data],[data length]               \n\
                i2c port options                                            \n\
                        0, 1, 2 or any port supported by the chip           \n\
                rate options (kHz)                                          \n\
                        100, 400, 1000                                      \n\
                mode options                                                \n\
                        0 (task), 1 (simple)                                \n\
                command options                                             \n\
                        write, read                                         \n\
                device address options (optional, default to 0x27)          \n\
                        uint32_t type in hexadecimal (case-insensitive)     \n\
                register options (for task mode only)                       \n\
                        uint32_t type in hexadecimal for digital register   \n\
                        operations (case-insensitive)                       \n\
                        uint16_t type in hexadecimal for other register     \n\
                        operations (case-insensitive)                       \n\
                register length options (size in bytes, for task mode only) \n\
                        1, 2, 4                                             \n\
                data options (for task mode only)                           \n\
                        uint8_t, uint16_t or uint32_t type                  \n\
                data length options (size in bytes, for task mode only)     \n\
                        1, 2, 4                                             \n\
EXAMPLES                                                                    \n\
        i2c,help                                                            \n\
                get help manual of i2c test command                         \n\
        i2c,1,100,1,write                                                   \n\
                send messages through i2c port 1, 100k speed, simple mode   \n\
        i2c,0,100,0,read,27,0,1,0,2                                         \n\
                receive messages through i2c port 0, 100k speed, task mode, \n\
                device address 0x27, register 0x00                        \n\n\
"

/**
 * \brief i2c test function
 * \param [in]  params  parameters received as strings from uart port
 */
void i2c_tester(const char **params);

#ifdef __cplusplus
}
#endif

#endif /* I2C_TESTER_EXT_H */