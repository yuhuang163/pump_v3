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

#include "nvrecord_bt.h"
#include "bes_me_api.h"
#include "adapter_service.h"

#include "android_test.h"

/****************************Macro defination*******************************/

/****************************Type defination********************************/
typedef struct
{
    uint8_t   bt_addr[6];
} android_basic_conn_env_t;
/****************************Function declaration***************************/

/****************************Variable defination****************************/
android_basic_conn_env_t basic_conn_env = {0};

/****************************Private function implementation****************/
static void basic_conn_enable_pairing(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    static const btif_access_mode_info_t info = {0x800, 0x12, 0x800, 0x12};

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    ret = bes_bt_me_set_accessible_mode(BTIF_BAM_GENERAL_ACCESSIBLE,&info);
    if (ret && (ret != BT_STS_PENDING))
    {
        LOG_E("enable pairing fail err=%d", ret);
        err = RESULT_BT_STACK_ERR;
    }
    else
    {
        err = RESULT_NO_ERR;
    }
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enable_pairing", "none", basic_conn_enable_pairing);

static void basic_conn_disable_pairing(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    static const btif_access_mode_info_t info = {0x800, 0x12, 0x800, 0x12};

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    ret = bes_bt_me_set_accessible_mode(BTIF_BAM_NOT_ACCESSIBLE, &info);
    if (ret && (ret != BT_STS_PENDING))
    {
        LOG_E("enable pairing fail err=%d", ret);
        err = RESULT_BT_STACK_ERR;
    }
    else
    {
        err = RESULT_NO_ERR;
    }
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "disable_pairing", "none", basic_conn_disable_pairing);

static void basic_conn_connect(int argc, char *argv[]);
static void bt_acl_conn_state_change(uint8_t type,bt_bdaddr_t* addr, uint32_t para)
{
    int err;

    if ((type == BT_ACL_CONNECT) &&
        (!memcmp(addr->address, basic_conn_env.bt_addr, 6)))
    {
        if (para)
        {
            err = RESULT_BT_STACK_ERR;
        }
        else
        {
            err = RESULT_NO_ERR;
        }
        memset(basic_conn_env.bt_addr, 0, 6);
        cmd_run_unlock(CMD_GET_FUNC_USER(basic_conn_connect));
        RESULT_PRINTF(err);
    }
}

static void basic_conn_connect(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_bdaddr_t bt_addr = {0};

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    cmd_char_to_hex(argv[1], bt_addr.address);
    bes_bt_me_acl_conn_state_callback_register(bt_acl_conn_state_change);
    ret = bt_adapter_create_bond(&bt_addr);
    if (ret)
    {
        err = RESULT_BT_STACK_ERR;
        RESULT_PRINTF(err);
        cmd_run_unlock(CMD_GET_USER());
    }
    else
    {
        memcpy(basic_conn_env.bt_addr, bt_addr.address, 6);
        err = RESULT_NO_ERR;
    }
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "connect", "none", basic_conn_connect);

static void bt_acl_disconn_state_change(uint8_t type,bt_bdaddr_t* addr, uint32_t para)
{
    int err;

    if ((type == BT_ACL_DISCONNECT) &&
        (!memcmp(addr->address, basic_conn_env.bt_addr, 6)))
    {
        if ((para) && (para != 0x16))
        {
            err = RESULT_BT_STACK_ERR;
        }
        else
        {
            err = RESULT_NO_ERR;
        }
        memset(basic_conn_env.bt_addr, 0, 6);
        cmd_run_unlock(CMD_GET_FUNC_USER(basic_conn_connect));
        RESULT_PRINTF(err);
    }
}

static void basic_conn_disconnect(int argc, char *argv[])
{
    int ret;
    int err = RESULT_BUSY_ERR;
    bt_bdaddr_t bt_addr = {0};

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    cmd_char_to_hex(argv[1], bt_addr.address);
    bes_bt_me_acl_conn_state_callback_register(bt_acl_disconn_state_change);
    ret = bt_adapter_disconnect_acl(&bt_addr);
    if (ret)
    {
        err = RESULT_BT_STACK_ERR;
        RESULT_PRINTF(err);
        cmd_run_unlock(CMD_GET_USER());
    }
    else
    {
        memcpy(basic_conn_env.bt_addr, bt_addr.address, 6);
        err = RESULT_NO_ERR;
    }
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "disconnect", "none", basic_conn_disconnect);

static void basic_conn_get_paired_device(int argc, char *argv[])
{
    bool ret;
    int err = RESULT_BUSY_ERR;
    uint8_t addr[6] = {0};
    char addr_string[13] = {0};
    nvrec_btdevicerecord *nv_dev = NULL;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    for (int i=0; i< BT_DEVICE_NUM; ++i)
    {
        ret = bes_bt_get_remote_dev_addr_by_device_id(i, addr);
        if (ret)
        {
            cmd_hex_to_char(addr, 6, addr_string);
            nv_record_btdevicerecord_find((bt_bdaddr_t *)addr, &nv_dev);
            RESULT_PRINTF_INFO("/***************DEV %d info***************/", i);
            RESULT_PRINTF_INFO("addr: %s", addr_string);
            RESULT_PRINTF_INFO("name: %s", nv_dev->record.remote_dev_name);
            RESULT_PRINTF_INFO("/*****************************************/");
        }
    }

    err = RESULT_NO_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "get_paired_device", "none", basic_conn_get_paired_device);

static void basic_conn_clear_paired_device(int argc, char *argv[])
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
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "clear_paired_device", "none", basic_conn_clear_paired_device);
