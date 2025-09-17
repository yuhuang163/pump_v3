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
#include "common.h"
#include "uart_cmd_test.h"
#include "app_trace_rx.h"
#ifdef SENSOR_HUB_BSP_TEST
#include "mcu_sensor_hub_app.h"
#endif
#include "audio_tester_ext.h"
#include "bt_tester_ext.h"
#include "flash_tester_ext.h"
#include "gpio_tester_ext.h"
#include "i2c_tester_ext.h"
#include "spi_tester_ext.h"
#include "wdt_tester_ext.h"
#include "common_debug_ext.h"

const uart_handler_t uart_test_func[] = {
    TEST_CMD_GROUP(audio)
    TEST_CMD_GROUP(bt)
    TEST_CMD_GROUP(flash)
    TEST_CMD_GROUP(gpio)
    TEST_CMD_GROUP(i2c)
    TEST_CMD_GROUP(spi)
    TEST_CMD_GROUP(wdt)
    DEBUG_CMD_GROUP(common)
};
ESHELL_TEST_COMMAND(spi);
ESHELL_TEST_COMMAND(wdt);
ESHELL_DEBUG_COMMAND(common);

WEAK_TEST_FUNC(audio)
WEAK_TEST_FUNC(bt)
WEAK_TEST_FUNC(flash)
WEAK_TEST_FUNC(gpio)
WEAK_TEST_FUNC(i2c)
WEAK_TEST_FUNC(spi)
WEAK_TEST_FUNC(wdt)
WEAK_DEBUG_FUNC(common)

WEAK void app_sensor_hub_at_cmd_send(uint8_t *buf, uint16_t len);

void cmd_split(char *cmd, char **params, uint32_t len)
{
    uint8_t i, j, offset;

    params[0] = cmd;
    for (i = j = offset = 0; i <= len; i++) {
        if (cmd[i] == CMD_DELIM) {
            params[j++][i - offset] = CMD_NULL;
            offset = i + 1;
            params[j] = cmd + offset;
        }
    }
    params[j][i- offset] = CMD_NULL;
}

uint32_t uart_cmd_callback(unsigned char *buf, uint32_t len)
{
    uint8_t i, j, offset;
    char *cmd = (char *)buf;
	char *params[CMD_ARG_NUM] = {NULL};
    size_t size = ARRAY_SIZE(uart_test_func);
    func_handler_t func_handler = NULL;

#ifdef SENSOR_HUB_BSP_TEST
    if (!strncmp(cmd, CMD_SENS, strlen(CMD_SENS))) {
        app_sensor_hub_at_cmd_send((uint8_t *)(cmd + strlen(CMD_SENS) + 1),
							(uint16_t)(strlen(cmd) - strlen(CMD_SENS) - 1));
        return 0;
    }
#endif

    for (offset = 0; cmd[offset] != CMD_DELIM; offset++)
        ;

    for (i = 0; i < size; i++) {
	    if (!strncmp(cmd, uart_test_func[i].name, offset)) {
		    func_handler = uart_test_func[i].handler;
            break;
        }
    }
    CMD_CHK(func_handler);

	cmd_split(cmd + offset + 1, &params[0], len - offset - 1);

    for (offset = i, i = 0; params[i]; i++) {
        for (j = 0; params[i][j] != CMD_NULL; j++) {
            CMD_CHK(CHAR_CHK(params[i][j]));
        }
    }

    if (!strncmp(params[0], CMD_HELP, strlen(CMD_HELP))) {
        HELP_INFO_PRINT_OUT(uart_test_func[offset].manual);
    } else {
        func_handler((const char **)&params[0]);
    }

    return 0;
}

void uart_cmd_init(void)
{
    CMD_TRACE("\"AT+BSP_TESTER\" registered");
    app_trace_rx_register("AT+BSP_TESTER", uart_cmd_callback);
}