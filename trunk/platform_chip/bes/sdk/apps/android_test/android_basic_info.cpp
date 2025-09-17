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

#include "factory_section.h"
#include "adapter_service.h"
#include "bes_me_api.h"
#include "bes_hfp_api.h"
#include "app_battery.h"
#include "ble_core_common.h"

#include "android_test.h"

/****************************Macro defination*******************************/

/****************************Type defination********************************/

/****************************Function declaration***************************/

/****************************Variable defination****************************/
static const char *dev_type_string[] =
{
    "HEADSET",
    "MOUSE",
    "KEYBOARD"
};

/****************************Private function implementation****************/
static void basic_info_get_device_info(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;
    char bt_addr_string[13] = {0};
    char ble_addr_string[13] = {0};
    uint8_t *bt_addr;
    uint8_t *ble_addr;
    uint8_t *bt_name;
    uint8_t *ble_name;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    bt_addr = factory_section_get_bt_address();
    cmd_hex_to_char(bt_addr, 6, bt_addr_string);
    ble_addr= factory_section_get_ble_address();
    cmd_hex_to_char(ble_addr, 6, ble_addr_string);
    RESULT_PRINTF_INFO("bt_addr: %s", bt_addr_string);
    RESULT_PRINTF_INFO("ble_addr: %s", ble_addr_string);

    bt_name = factory_section_get_bt_name();
    ble_name = factory_section_get_ble_name();
    RESULT_PRINTF_INFO("bt_name: %s", bt_name);
    RESULT_PRINTF_INFO("ble_name: %s", ble_name);

    err= RESULT_NO_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "get_device_info", "none", basic_info_get_device_info);

static void basic_info_set_name(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    bes_bt_me_set_bt_name(argv[1], strlen(argv[1]));
    bes_bt_me_set_ble_name(argv[2], strlen(argv[2]));
    bes_ble_init_ble_name(argv[2]);

    factory_section_set_bt_name(argv[1], strlen(argv[1])+1);
    factory_section_set_ble_name(argv[2]);

    err= RESULT_NO_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "set_name", "none", basic_info_set_name);

static void basic_info_set_address(int argc, char *argv[])
{
    int param[6] = {0};
    uint8_t bt_addr[6] = {0};

    sscanf(argv[1], "%x:%x:%x:%x:%x:%x", &param[0],
        &param[1] , &param[2], &param[3], &param[4], &param[5]);
    bt_addr[0] = param[0];
    bt_addr[1] = param[1];
    bt_addr[2] = param[2];
    bt_addr[3] = param[3];
    bt_addr[4] = param[4];
    bt_addr[5] = param[5];
    LOG_I("set_address, addr=%x:%x:%x:%x:%x:%x\r\n",
        bt_addr[0], bt_addr[1], bt_addr[2], bt_addr[3],bt_addr[4], bt_addr[5]);

    factory_section_set_bt_address(bt_addr);
    factory_section_set_ble_address(bt_addr);

    bes_bt_me_set_bt_address(bt_addr);
    bes_bt_me_set_ble_address(bt_addr);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "set_address", "none", basic_info_set_address);

static void basic_info_get_device_type(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    RESULT_PRINTF_INFO("dev_type: %s", dev_type_string[0]);
    err = RESULT_NO_ERR;
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "get_device_type", "none", basic_info_get_device_type);

static void basic_info_set_device_type(int argc, char *argv[])
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
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "set_device_type", "none", basic_info_set_device_type);

static void basic_info_get_true_wireless(int argc, char *argv[])
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
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "get_true_wireless", "none", basic_info_get_true_wireless);

static void basic_info_set_true_wireless(int argc, char *argv[])
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
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "set_true_wireless", "none", basic_info_set_true_wireless);

static void basic_info_get_battery_level(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;
    uint8_t currlevel;
    APP_BATTERY_MV_T currvolt;
    enum APP_BATTERY_STATUS_T status;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    app_battery_get_info(&currvolt, &currlevel, &status);
    RESULT_PRINTF_INFO("battery_level: %d", (currlevel+1)*10);
    err = RESULT_NO_ERR;
    cmd_run_unlock(CMD_GET_USER());


    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "get_battery_level", "none", basic_info_get_battery_level);

static void basic_info_set_battery_level(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;
    uint8_t battery_level;
    int battery;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    cmd_param_parse(argc, argv, &battery);
    if ((battery > 100) || (battery < 0))
    {
        err = RESULT_PARAM_ERR;
    }
    else
    {
        battery_level = battery / 10;
        battery_level = battery_level? (battery_level-1) : 0;
        bes_bt_hfp_set_battery_level(battery_level);
        err = RESULT_NO_ERR;
    }
    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "set_battery_level", "none", basic_info_set_battery_level);