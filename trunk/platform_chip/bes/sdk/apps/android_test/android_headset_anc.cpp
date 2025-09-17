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
#include <stdio.h>
#include <string.h>


#include "android_test.h"

/****************************Macro defination*******************************/

/****************************Type defination********************************/

/****************************Function declaration***************************/

/****************************Variable defination****************************/

/****************************Private function implementation****************/
static void anc_on(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    err = RESULT_NOT_SUPPORT_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "anc_on", "none", anc_on);

static void anc_off(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    err = RESULT_NOT_SUPPORT_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "anc_off", "none", anc_off);

static void get_anc_mode(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    err = RESULT_NOT_SUPPORT_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "get_anc_mode", "none", get_anc_mode);

static void set_anc_mode(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    err = RESULT_NOT_SUPPORT_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "set_anc_mode", "none", set_anc_mode);