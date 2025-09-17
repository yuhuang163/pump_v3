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
#ifndef SPI_TEST_H
#define SPI_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "hal_spi.h"

typedef void (*SPI_TESTER_HANDLER_T)(void);

typedef struct {
    const char *                name;
    SPI_TESTER_HANDLER_T        handler;
}spi_tester_handler_t;

#ifdef __cplusplus
}
#endif

#endif /* SPI_TEST_H */