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

#include "pmu.h"
#include "nvrecord_bt.h"
#include "nvrecord_ble.h"

#include "android_test.h"

/****************************Macro defination*******************************/
#define ANDROID_TEST_TIMEOUT_MS  10000
#define ANDROID_TEST_TIME_NAME "android_test_timer"

/****************************Type defination********************************/
typedef struct
{
    bool        runing;
    const char  *lock_user;
    osTimerId_t timer_id;
    osTimerAttr_t timer_attr;
} android_test_env_t;

/****************************Function declaration***************************/

/****************************Variable defination****************************/
#ifdef ESHELL_ON_TRACE
char log_buf[RESULT_INFO_MAX_LEN] = RESULT_TAG_INFO;
#endif
android_test_env_t test_env = {0};

/****************************Private function implementation****************/
void cmd_param_parse(int argc, char *argv[], int *data)
{
    int param_num = argc-1;

    LOG_I("[test] param_num=%d\r\n", param_num);
    for (int i=0; i<param_num; ++i)
    {
        sscanf(argv[i+1], "%d", data+i);
    }
}

bool cmd_char_to_hex(char* string, uint8_t *hex)
{
    int n = 0;

    if ((!string) || (!hex))
    {
        return 0;
    }

    for (int i = 0; string[i]; i += 2)
    {
        if (string[i] >= 'A' && string[i] <= 'F')
        {
            hex[n] = string[i] - 'A' + 10;
        }
        else if (string[i] >= 'a' && string[i] <= 'f')
        {
            hex[n] = string[i] - 'a' + 10;
        }
        else if (string[i] >= '0' && string[i] <= '9')
        {
            hex[n] = string[i] - '0';
        }

        if (string[i+1] >= 'A' && string[i+1] <= 'F')
        {
            hex[n] = (hex[n] << 4) | (string[i+1] - 'A' + 10);
        }
        else if (string[i+1] >= 'a' && string[i+1] <= 'f')
        {
            hex[n] = (hex[n] << 4) | (string[i+1] - 'a' + 10);
        }
        else if (string[i+1] >= '0' && string[i+1] <= '9')
        {
            hex[n] = (hex[n] << 4) | (string[i+1] - '0');
        }
        ++n;
    }

    return true;
}

int cmd_hex_to_char(const uint8_t *hex, uint8_t hex_len, char *string)
{
    int n = 0;
    uint8_t temp_data = 0;
    uint8_t temp_hex[16] = {0};

    memcpy(temp_hex, hex, hex_len);
    for (int i=0; i < hex_len; ++i)
    {
        temp_data = temp_hex[i] >> 4;
        if (temp_data<10)
        {
            string[n] = temp_data + '0';
        }
        else
        {
            string[n] = (temp_data-10) + 'A';
        }
        n++;

        temp_data = temp_hex[i] & 0x0F;
        if (temp_data<10)
        {
            string[n] = temp_data + '0';
        }
        else
        {
            string[n] = (temp_data-10) + 'A';
        }
        n++;
    }

    return n;
}

void cmd_run_timeout_handle(void* p_env)
{
    int err = RESULT_TIMEOUT_ERR;

    LOG_E("cmd_timeout user=%s", test_env.lock_user);
    RESULT_PRINTF(err);
    cmd_run_unlock(test_env.lock_user);

}

bool cmd_run_lock(const char *user)
{
    if (test_env.runing)
    {
        return false;
    }

    if (!test_env.timer_id)
    {
        test_env.timer_attr.name = ANDROID_TEST_TIME_NAME;
        test_env.timer_id = osTimerNew(cmd_run_timeout_handle, osTimerOnce,
            NULL, &test_env.timer_attr);
    }
    osTimerStart(test_env.timer_id, ANDROID_TEST_TIMEOUT_MS);
    test_env.lock_user = user;
    test_env.runing    = 1;
    return true;
}

bool cmd_run_unlock(const char *user)
{
    if (!test_env.runing)
    {
        return false;
    }

    if (user != test_env.lock_user)
    {
        return false;
    }

    osTimerStop(test_env.timer_id);
    test_env.lock_user = NULL;
    test_env.runing = 0;
    return true;
}

static void basic_reboot(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    err = RESULT_NO_ERR;
    RESULT_PRINTF(err);
    LOG_I("Cmd Reboot\n");
    pmu_reboot();

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "reboot", "none", basic_reboot);

static void basic_factory_reset(int argc, char *argv[])
{
    int err = RESULT_BUSY_ERR;

    if (!cmd_run_lock(CMD_GET_USER()))
    {
        RESULT_PRINTF(err);
        return;
    }

    err = RESULT_NO_ERR;
    nv_record_ddbrec_clear_all_bt_paired_list();
    nv_record_ble_delete_all_entry();

    cmd_run_unlock(CMD_GET_USER());

    RESULT_PRINTF(err);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "factory_reset", "none", basic_factory_reset);