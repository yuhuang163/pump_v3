/**
 * @copyright Copyright (c) 2015-2023 BES Technic.
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
 */
/*****************************header include********************************/
#include <string.h>
#include "hal_trace.h"
#include "eshell.h"

/****************************Macro defination*******************************/
// Auto test debug serial port
#ifdef ESHELL_ON_TRACE
extern char log_buf[];
#define RESULT_INFO_MAX_LEN 30
#define RESULT_TAG_INFO "[MOBLY_TEST]:"
#define RESULT_STRING(res) (res? "FAIL":"SUCCESS")
#define RESULT_PRINTF(err)  TRACE(0, "%sresult: %s, error_code=%d", RESULT_TAG_INFO, RESULT_STRING(err), err)
#define RESULT_PRINTF_DATA(data, data_len) DUMP8(data, data_len)
#define RESULT_PRINTF_INFO(fmt, ...)  do{\
                                        strcpy(log_buf+strlen(RESULT_TAG_INFO), fmt);\
                                        TRACE(0, log_buf, ##__VA_ARGS__);\
                                        }while(0)
#else
#define RESULT_STRING(res) (res? "FAIL":"SUCCESS")
#define RESULT_PRINTF(err)  eshell_putstring_nl("result: %s, error_code=%d", RESULT_STRING(err), err)
#define RESULT_PRINTF_INFO(fmt, ...)  eshell_putstring_nl(fmt, ##__VA_ARGS__)
#define RESULT_PRINTF_DATA(data, data_len) eshell_output_rawdata(data, data_len)
#endif
#define MODULE_TRACE_LEVEL  TR_LEVEL_DEBUG
#define LOG_V(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_VERBOSE) TRACE(0, str, ##__VA_ARGS__)
#define LOG_D(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_DEBUG)   TRACE(0, str, ##__VA_ARGS__)
#define LOG_I(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_INFO)    TRACE(0, str, ##__VA_ARGS__)
#define LOG_W(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_WARN)    TRACE(0, str, ##__VA_ARGS__)
#define LOG_E(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_ERROR)   TRACE(0, str, ##__VA_ARGS__)

// Printf result string
#define CMD_GET_USER()             (__func__)
#define CMD_GET_FUNC_USER(func)    (#func)

/****************************Type defination********************************/
enum RESULT_ERROE_CODE_E{
    RESULT_NO_ERR,
    RESULT_BUSY_ERR,
    RESULT_PARAM_ERR,
    RESULT_NOT_SUPPORT_ERR,
    RESULT_TIMEOUT_ERR,
    RESULT_BT_STACK_ERR,
};
/****************************Function declaration***************************/

/****************************Variable defination****************************/

/****************************Private function implementation****************/
void cmd_param_parse(int argc, char *argv[], int *data);

bool cmd_char_to_hex(char* string, uint8_t *hex);

int cmd_hex_to_char(const uint8_t *hex, uint8_t hex_len, char *string);

bool cmd_run_lock(const char *user);

bool cmd_run_unlock(const char *user);

