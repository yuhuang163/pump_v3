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

#include "adapter_service.h"
#include "hfp_common_define.h"
#include "bes_hfp_api.h"

#include "android_test.h"

/****************************Macro defination*******************************/

/****************************Type defination********************************/

/****************************Function declaration***************************/

/****************************Variable defination****************************/

/****************************Private function implementation****************/
static void call_accept(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_hfp_state_t dev_hfp;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

#ifdef BT_HFP_SUPPORT
    err = RESULT_NO_ERR;
    for (int i=0; i< BT_DEVICE_NUM; ++i)
    {
        dev_hfp = bes_bt_hfp_get_state(i);
        if ((dev_hfp.hfp_chan_state == BT_HFP_CHAN_STATE_OPEN)
            &&(dev_hfp.callsetup_state == BT_HFP_CALL_SETUP_IN))
        {
            ret = bes_bt_hfp_call_action(i, BT_HFP_ANSWER_CALL);
            if (ret && (ret != BT_STS_PENDING))
            {
                LOG_E("call_accept fail err=%d", ret);
                err = RESULT_BT_STACK_ERR;
                break;
            }
            else
            {
                err = RESULT_NO_ERR;
            }
        }
    }
#else
    err = RESULT_NOT_SUPPORT_ERR;
#endif
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "call_accept", "none", call_accept);

static void call_decline(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_hfp_state_t dev_hfp;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

#ifdef BT_HFP_SUPPORT
    err = RESULT_NO_ERR;
    for (int i=0; i< BT_DEVICE_NUM; ++i)
    {
        dev_hfp = bes_bt_hfp_get_state(i);
        if (dev_hfp.call_is_active ||
           ((dev_hfp.hfp_chan_state == BT_HFP_CHAN_STATE_OPEN)
           && (dev_hfp.callsetup_state == BT_HFP_CALL_SETUP_IN)))
        {
            ret = bes_bt_hfp_call_action(i, BT_HFP_HANGUP_CALL);
            if (ret && (ret != BT_STS_PENDING))
            {
                LOG_E("call_decline fail err=%d", ret);
                err = RESULT_BT_STACK_ERR;
                break;
            }
            else
            {
                err = RESULT_NO_ERR;
            }
        }
   }
#else
    err = RESULT_NOT_SUPPORT_ERR;
#endif
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "call_decline", "none", call_decline);

static void call_hold(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_hfp_state_t dev_hfp;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

#ifdef BT_HFP_SUPPORT
    err = RESULT_NO_ERR;
    for (int i=0; i < BT_DEVICE_NUM; ++i)
    {
        dev_hfp = bes_bt_hfp_get_state(i);
        if (dev_hfp.call_is_active)
        {
            ret = bes_bt_hfp_call_hold(i, BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, i);
            if (ret && (ret != BT_STS_PENDING))
            {
                LOG_E("call_hold fail err=%d", ret);
                err = RESULT_BT_STACK_ERR;
                break;
            }
            else
            {
                err = RESULT_NO_ERR;
            }
        }
    }
#else
    err = RESULT_NOT_SUPPORT_ERR;
#endif
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "call_hold", "none", call_hold);

static void call_redial(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_hfp_state_t dev_hfp;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

#ifdef BT_HFP_SUPPORT
    err = RESULT_NO_ERR;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        dev_hfp = bes_bt_hfp_get_state(i);
        if (dev_hfp.hfp_chan_state == BT_HFP_CHAN_STATE_OPEN)
        {
            ret = bes_bt_hfp_call_action(i, BT_HFP_REDIAL_CALL);
            if (ret && (ret != BT_STS_PENDING))
            {
                LOG_E("call_redial err=%d", ret);
                err = RESULT_BT_STACK_ERR;
                break;
            }
            else
            {
                err = RESULT_NO_ERR;
            }
        }
    }
#else
    err = RESULT_NOT_SUPPORT_ERR;
#endif
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "call_redial", "none", call_redial);
