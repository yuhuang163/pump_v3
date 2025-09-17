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
#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#ifdef UTILS_ESHELL_EN
#include "eshell.h"
#endif
#include CHIP_SPECIFIC_HDR(hal_iomux)
#include CHIP_SPECIFIC_HDR(reg_cmu)
#include CHIP_SPECIFIC_HDR(reg_aoncmu)
#include CHIP_SPECIFIC_HDR(reg_aonsec)
#include CHIP_SPECIFIC_HDR(reg_btcmu)
#include CHIP_SPECIFIC_HDR(reg_senscmu)

#define CONCAT_IMPL(a, b)       a ## b
#define CONCAT(a, b)            CONCAT_IMPL(a, b)

#define CONVERSION_BASE         16
#define CMD_ARG_NUM             20
#define CMD_DELIM               ','
#define CMD_NULL                '\0'
#define CMD_SENS                TO_STRING(sens)
#define CMD_HELP                TO_STRING(help)
#define ERR_CMD_MSG             TO_STRING(invalid command)
#define ERR_PARAM_MSG           TO_STRING(invalid parameters)
#define ERR_FUNC_MSG            TO_STRING(not supported)
#define ASSIGN_VAL(n, v)                                                    \
        if (1) {                                                            \
            if (v) {                                                        \
                n = (typeof(n))atoi(v);                                     \
            }                                                               \
        } else ((void)0)
#define STR_TO_HEX(n, v)                                                    \
        if (1) {                                                            \
            uint32_t tmp1 = 0, tmp2 = 0;                                    \
            char buf[10] = {0};                                             \
            if (v) {                                                        \
                for (uint8_t i = 0; i < strlen(v); i++) {                   \
                    PARAM_CHK(HEX_CHK(v[i]));                               \
                }                                                           \
                if (strlen(v) == 8) {                                       \
                    memcpy(buf, v, 4);                                      \
                    buf[4] = CMD_NULL;                                      \
                    tmp1 = (uint32_t)strtol(buf, NULL, CONVERSION_BASE);    \
                    memcpy(buf, &v[4], 4);                                  \
                    buf[4] = CMD_NULL;                                      \
                    tmp2 = (uint32_t)strtol(buf, NULL, CONVERSION_BASE);    \
                    n = (typeof(n))((tmp1 << 16) | tmp2);                   \
                } else {                                                    \
                    n = (typeof(n))strtol(v, NULL, CONVERSION_BASE);        \
                }                                                           \
            }                                                               \
        } else ((void)0)

#define TEST_CMD(n)             CONCAT(n, _tester)
#define DEBUG_CMD(n)            CONCAT(n, _debug)
#define ESHELL_TEST_CMD(n)      CONCAT( TEST_CMD(n), _eshell)
#define ESHELL_DEBUG_CMD(n)     CONCAT(DEBUG_CMD(n), _eshell)
#define HELP_INFO(n)            CONCAT(n, _CMD_HELP_INFO)
#define TEST_CMD_GROUP(n)       {TO_STRING( TEST_CMD(n)), HELP_INFO(n),  TEST_CMD(n)},
#define DEBUG_CMD_GROUP(n)      {TO_STRING(DEBUG_CMD(n)), HELP_INFO(n), DEBUG_CMD(n)},
#define HELP_INFO_PRINT_OUT(n)  TRACE_OUTPUT((const unsigned char*)(n), strlen((const char*)(n)))
#define WEAK_FUNC_DEFINE(n)     WEAK void n(const char **params) {CMD_TRACE(ERR_FUNC_MSG);}
#define WEAK_TEST_FUNC(n)       WEAK_FUNC_DEFINE( TEST_CMD(n))
#define WEAK_DEBUG_FUNC(n)      WEAK_FUNC_DEFINE(DEBUG_CMD(n))

#define ESHELL_HELP_INFO(n)     if (a < 2) return; \
                                if (!strncmp(argv[1], CMD_HELP, strlen(CMD_HELP)))  \
                                    HELP_INFO_PRINT_OUT(HELP_INFO(n)); else
#define ESHELL_TEST_FUNC(n)     void ESHELL_TEST_CMD(n) (int a, char **argv)\
                                {   ESHELL_HELP_INFO(n)                     \
                                     TEST_CMD(n)((const char **)&argv[1]); }
#define ESHELL_DEBUG_FUNC(n)    void ESHELL_DEBUG_CMD(n)(int a, char **argv)\
                                {   ESHELL_HELP_INFO(n)                     \
                                    DEBUG_CMD(n)((const char **)&argv[1]); }
#define ESHELL_TEST_COMMAND(n)  ESHELL_TEST_FUNC(n)                         \
                                ESHELL_DEF_COMMAND( ESHELL_CMD_GRP_BSP_TEST,\
                                                    TO_STRING(n),           \
                                                    HELP_INFO(n),           \
                                                    n ## _tester_eshell)
#define ESHELL_DEBUG_COMMAND(n) ESHELL_DEBUG_FUNC(n)                        \
                                ESHELL_DEF_COMMAND( ESHELL_CMD_GRP_BSP_TEST,\
                                                    TO_STRING(n),           \
                                                    HELP_INFO(n),           \
                                                    n ##  _debug_eshell)

#define CMD_TRACE(f, ...)       TRACE_IMM(0, "[%s] " f, __func__, ##__VA_ARGS__)
#define CMD_DEBUG_TRACE(f, ...) TRACE_IMM(0, "[%s][%d] " f, __func__, __LINE__, ##__VA_ARGS__)

#define NUM_CHK(n)              (((n) >= '0') && ((n) <= '9'))
#define LOWER_CHAR_CHK(n)       (((n) >= 'a') && ((n) <= 'z'))
#define UPPER_CHAR_CHK(n)       (((n) >= 'A') && ((n) <= 'Z'))
#define CHAR_CHK(n)             (((n) == '_') || NUM_CHK(n) || LOWER_CHAR_CHK(n) || UPPER_CHAR_CHK(n))
#define HEX_CHK(n)              ((((n) >= 'a') && ((n) <= 'f')) ||          \
                                 (((n) >= 'A') && ((n) <= 'F')) || NUM_CHK(n))
#define PIN_CHK(n)              ((((n) % 10) >= 0) && (((n) % 10) <  8) &&  \
                                 (((n) / 10) >= 0) && (((n) / 10) < 10))
#define REG_CHK(r)              ((((r) >=          CMU_BASE) && ((r) <=          CMU_BASE + sizeof(struct     CMU_T))) ||   \
                                 (((r) >=      AON_CMU_BASE) && ((r) <=      AON_CMU_BASE + sizeof(struct  AONCMU_T))) ||   \
                                 (((r) >= AON_SEC_CTRL_BASE) && ((r) <= AON_SEC_CTRL_BASE + sizeof(struct  AONSEC_T))) ||   \
                                 (((r) >=       BT_CMU_BASE) && ((r) <=       BT_CMU_BASE + sizeof(struct   BTCMU_T))) ||   \
                                 (((r) >=     SENS_CMU_BASE) && ((r) <=     SENS_CMU_BASE + sizeof(struct SENSCMU_T))))
#define CMD_CHK(cond)                                                       \
        if (1) {                                                            \
            if (!(cond)) {                                                  \
                CMD_DEBUG_TRACE("%s", ERR_CMD_MSG);                         \
                return 0;                                                   \
            }                                                               \
        } else ((void)0)
#define PARAM_CHK(cond)                                                     \
        if (1) {                                                            \
            if (!(cond)) {                                                  \
                CMD_DEBUG_TRACE("%s", ERR_PARAM_MSG);                       \
                return;                                                     \
            }                                                               \
        } else ((void)0)
#define CMD_HANDLER(h, n, ...)                                              \
        if (1) {                                                            \
            bool flag = false;                                              \
            for (int i = 0; i < ARRAY_SIZE(CONCAT(h, _handler)); i++) {     \
                if (!strncmp(CONCAT(h, _handler)[i].name, n,                \
                      strlen(CONCAT(h, _handler)[i].name))) {               \
                    flag = true;                                            \
                    CONCAT(h, _handler)[i].handler(__VA_ARGS__);            \
                    break;                                                  \
                }                                                           \
            }                                                               \
            PARAM_CHK(flag);                                                \
        } else ((void)0)

#endif /* TEST_COMMON_H */