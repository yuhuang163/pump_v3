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
#ifndef UART_CMD_TEST_H
#define UART_CMD_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*func_handler_t)(const char **params);

typedef struct {
    const char *                name;
    const char *                manual;
    func_handler_t              handler;
} uart_handler_t;

uint32_t uart_cmd_callback(unsigned char *buf, uint32_t len);

void uart_cmd_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_CMD_TEST_H */