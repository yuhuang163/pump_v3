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
#ifndef I2C_TEST_H
#define I2C_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "hal_i2c.h"

typedef void (*I2C_TESTER_HANDLER_T)(void);

typedef struct {
    const char *                name;
    I2C_TESTER_HANDLER_T        handler;
}i2c_tester_handler_t;

#ifdef __cplusplus
}
#endif

#endif /* I2C_TEST_H */