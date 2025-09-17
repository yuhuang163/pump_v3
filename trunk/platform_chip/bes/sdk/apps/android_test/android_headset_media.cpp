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

#include "hal_aud.h"

#include "adapter_service.h"
#include "bes_me_api.h"
#include "bes_avrcp_api.h"
#include "bes_a2dp_api.h"

#include "android_test.h"

/****************************Macro defination*******************************/

/****************************Type defination********************************/

/****************************Function declaration***************************/

/****************************Variable defination****************************/

/****************************Private function implementation****************/
static void media_play(int argc, char *argv[])
{
    int ret;
    int err = RESULT_NO_ERR;
    bt_bdaddr_t remote_addr;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    for (int i=0; i< BT_DEVICE_NUM; ++i)
    {
        ret = bes_bt_get_remote_dev_addr_by_device_id(i, (uint8_t *)&remote_addr);
        if (ret)
        {
            ret = bes_bt_avrcp_send_play_cmd(&remote_addr);

            if (ret && (ret != BT_STS_PENDING))
            {
                LOG_E("enable pairing fail err=%d", ret);
                err = RESULT_BT_STACK_ERR;
            }
            else
            {
                err = RESULT_NO_ERR;
            }
        }
    }
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "media_play", "none", media_play);

static void media_pause(int argc, char *argv[])
{
    int ret;
    int err = RESULT_NO_ERR;
    bt_bdaddr_t remote_addr;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    for (int i=0; i< BT_DEVICE_NUM; ++i)
    {
        ret = bes_bt_get_remote_dev_addr_by_device_id(i, (uint8_t *)&remote_addr);
        if (ret)
        {
            ret = bes_bt_avrcp_send_pause_cmd(&remote_addr);

            if (ret && (ret != BT_STS_PENDING))
            {
                LOG_E("enable pairing fail err=%d", ret);
                err = RESULT_BT_STACK_ERR;
            }
            else
            {
                err = RESULT_NO_ERR;
            }
        }
    }
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "media_pause", "none", media_pause);

static void media_next(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_bdaddr_t remote_addr;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    for (int i=0; i< BT_DEVICE_NUM; ++i)
    {
        ret = bes_bt_get_remote_dev_addr_by_device_id(i, (uint8_t *)&remote_addr);
        if (ret)
        {
            ret = bes_bt_avrcp_key_operation(&remote_addr, BES_BT_AVRCP_POP_FORWARD, 1);

            if (ret && (ret != BT_STS_PENDING))
            {
                LOG_E("enable pairing fail err=%d", ret);
                err = RESULT_BT_STACK_ERR;
            }
            else
            {
                err = RESULT_NO_ERR;
            }
        }
    }
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "media_next", "none", media_next);

static void media_prev(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_bdaddr_t remote_addr;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    for (int i=0; i< BT_DEVICE_NUM; ++i)
    {
        ret = bes_bt_get_remote_dev_addr_by_device_id(i, (uint8_t *)&remote_addr);
        if (ret)
        {
            ret = bes_bt_avrcp_key_operation(&remote_addr, BES_BT_AVRCP_POP_BACKWARD, 1);

            if (ret && (ret != BT_STS_PENDING))
            {
                LOG_E("enable pairing fail err=%d", ret);
                err = RESULT_BT_STACK_ERR;
            }
            else
            {
                err = RESULT_NO_ERR;
            }
        }
    }
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "media_prev", "none", media_prev);

static void media_get_volume(int argc, char *argv[])
{
    uint8_t vol = 0;
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    vol = bes_bt_a2dp_get_bt_vol(BT_DEVICE_ID_1);
    RESULT_PRINTF_INFO("volume=%d, (range:0-127)", vol);
    err = RESULT_NO_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "get_volume", "none", media_get_volume);

static void media_set_volume(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;
    int volume = 0;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    cmd_param_parse(argc, argv, &volume );
    for (int i=0; i< BT_DEVICE_NUM; ++i)
    {
        bes_bt_a2dp_set_volume(i, volume);
        bes_bt_a2dp_abs_volume_mix_version_handled(i);
        err = RESULT_NO_ERR;
    }
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "set_volume", "none", media_set_volume);

static void media_volume_plus(int argc, char *argv[])
{
    uint8_t vol;
    int ret = 0;
    int err = RESULT_BUSY_ERR;
    bt_bdaddr_t remote_addr;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    vol = bes_bt_a2dp_get_local_vol(BT_DEVICE_ID_1);
    if(vol <= TGT_VOLUME_LEVEL_MAX)
    {
        vol++;
        for (int i=0; i< BT_DEVICE_NUM; ++i)
        {
            ret = bes_bt_get_remote_dev_addr_by_device_id(i, (uint8_t *)&remote_addr);
            if (ret)
            {
                bes_bt_a2dp_set_local_vol(i, vol);
                bes_bt_a2dp_abs_volume_mix_version_handled(i);
            }
        }
    }
    err = RESULT_NO_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "volume_plus", "none", media_volume_plus);

static void media_volume_dec(int argc, char *argv[])
{
    uint8_t vol;
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_bdaddr_t remote_addr;


    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    vol = bes_bt_a2dp_get_local_vol(BT_DEVICE_ID_1);
    if(vol > TGT_VOLUME_LEVEL_MUTE)
    {
        vol--;
        for (int i=0; i< BT_DEVICE_NUM; ++i)
        {
            ret = bes_bt_get_remote_dev_addr_by_device_id(i, (uint8_t *)&remote_addr);
            if (ret)
            {
                bes_bt_a2dp_set_local_vol(i, vol);
                bes_bt_a2dp_abs_volume_mix_version_handled(i);
            }
        }
    }
    err = RESULT_NO_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "volume_dec", "none", media_volume_dec);