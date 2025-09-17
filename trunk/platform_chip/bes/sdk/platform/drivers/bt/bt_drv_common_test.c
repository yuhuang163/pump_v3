/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "plat_types.h"
#include "hal_chipid.h"
#include "bt_drv.h"
#include "bt_drv_internal.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "besbt_string.h"
#include "bt_drv_common_test_case.h"
#include "hal_sysfreq.h"

void btdrv_common_pscan_standard_enable_test(uint16_t interval,uint16_t window)
{
    btdrv_common_test_cmd_wr_pagescan_type(0);//standard
    btdrv_common_test_cmd_wr_pscan_activity(interval,window);
    btdrv_common_test_cmd_scan_control(BTDRV_COMMON_TEST_PAGE_SCAN_ENABLE);
}

void btdrv_common_pscan_interlaced_enable_test(uint16_t interval,uint16_t window)
{
    btdrv_common_test_cmd_wr_pagescan_type(1);//interlaced
    btdrv_common_test_cmd_wr_pscan_activity(interval,window);
    btdrv_common_test_cmd_scan_control(BTDRV_COMMON_TEST_PAGE_SCAN_ENABLE);
}

//bt connection slave side
void btdrv_common_bothscan_enable_test(uint16_t interval,uint16_t window)
{
    uint8_t local_addr[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    btdrv_common_test_cmd_wr_bt_addr(local_addr);
    btdrv_common_test_cmd_set_link_policy(5);//allow switch&sniff
    btdrv_common_test_cmd_wr_pscan_activity(interval,window);
    btdrv_common_test_cmd_scan_control(BTDRV_COMMON_TEST_BOTH_SCAN_ENABLE);
    btdrv_common_test_cmd_autoaccept_connect();
}

//bt connection master side
void btdrv_common_bt_create_connect_test(void)
{
    uint8_t local_addr[6] = {0x12,0x34,0x56,0x78,0x90,0xab};
    uint8_t peer_addr[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    btdrv_common_test_cmd_wr_bt_addr(local_addr);
    btdrv_common_test_cmd_set_link_policy(5);//allow switch&sniff
    btdrv_common_test_cmd_bt_create_connect(peer_addr,0xcc18,0x02,0x0000,1);
}

void btdrv_common_bt_enter_sniff_test(uint16_t conhandle,uint16_t sniff_interval,uint16_t attempt,uint16_t timeout)
{
    btdrv_common_test_cmd_enter_sniff_mode(conhandle,sniff_interval,sniff_interval,attempt,timeout);
}

//ble connection slave side
void btdrv_common_ble_adv_enable_test(uint16_t adv_interval)
{
    uint8_t local_addr[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    uint8_t peer_addr[6] = {0};
    btdrv_common_test_cmd_wr_ble_pubaddr(local_addr);
    btdrv_common_test_cmd_ble_set_adv_param(adv_interval,adv_interval,0,0,0,peer_addr,7,0);
    btdrv_common_test_cmd_enable_ble_adv(1);
}

//ble connection master side
void btdrv_common_ble_create_connect_test(uint16_t conn_interval)
{
    uint8_t local_addr[6] = {0x12,0x34,0x56,0x78,0x90,0xab};
    uint8_t peer_addr[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    btdrv_common_test_cmd_wr_ble_pubaddr(local_addr);
    btdrv_common_test_cmd_le_create_connect(0x0050,0x0030,0,0,peer_addr,0,conn_interval,conn_interval,0,0x0064,0x0010,0x0010);
}

typedef struct adv_data_t{
    uint8_t data[31];
    uint8_t len;
}adv_data_t;

typedef struct adv_element {
    // Length of Type and Value fields for AD data type
    uint8_t len;
    // Type: «Service Data - 16-bit UUID»
    uint8_t type;
    uint8_t data[31];
}adv_data_elem_t;

void btdrv_common_adv_data_add_element(adv_data_t *adv_param, uint8_t adv_type,
                             uint16_t uuid_type, const uint8_t *adv_data, uint8_t adv_len)
{
    adv_data_elem_t part_data;

    part_data.type = adv_type;
    if(uuid_type != 0){       //Contains UUID data
        part_data.data[0] = uuid_type & 0x00FF;
        part_data.data[1] = (uuid_type & 0xFF00) >> 8;
        memcpy(&part_data.data[2], adv_data, adv_len);
        part_data.len =  sizeof(part_data.type) + sizeof(uuid_type)+ adv_len;
    }else{                    //no contains UUID data
        memcpy(part_data.data, adv_data, adv_len);
        part_data.len =  sizeof(part_data.type) + adv_len;
    }

    memcpy(&adv_param->data[adv_param->len], &part_data,
              part_data.len+sizeof(part_data.len));

    adv_param->len += part_data.len+sizeof(part_data.len);
}

struct adv_data_t btdrv_common_adv_info;
#define AD_TYPE_FLAGS    0x01
/// Complete device name
#define AD_TYPE_COMPLETE_NAME    0x09
const char *BLE_TEST_NAME = "BES_BLE";
void btdrv_common_set_default_adv_data(void)
{
    uint8_t adv_flgs = 0x1A;
    uint8_t adv_name[20] = {0};

    uint32_t nameLen = strlen(BLE_TEST_NAME);
    memcpy(adv_name, BLE_TEST_NAME, nameLen);

    btdrv_common_adv_data_add_element(&btdrv_common_adv_info, AD_TYPE_FLAGS, 0, &adv_flgs, sizeof(adv_flgs));
    btdrv_common_adv_data_add_element(&btdrv_common_adv_info, AD_TYPE_COMPLETE_NAME, 0, adv_name, nameLen);
}

void btdrv_common_ble_adv_enable_with_data_test(uint16_t adv_interval)
{
    uint8_t local_addr[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    uint8_t peer_addr[6] = {0};
    btdrv_common_test_cmd_wr_ble_pubaddr(local_addr);
    btdrv_common_test_cmd_ble_set_adv_param(adv_interval,adv_interval,0,0,0,peer_addr,7,0);
    btdrv_common_set_default_adv_data();
    btdrv_common_test_cmd_set_ble_adv_data(btdrv_common_adv_info.len,btdrv_common_adv_info.data);
    btdrv_common_test_cmd_enable_ble_adv(1);
}

void btdrv_common_ble_scan_enable(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window)
{
    btdrv_common_test_cmd_set_ble_scan_param(scan_type,scan_interval,scan_window,0,1);
    btdrv_common_test_cmd_ble_scan_en();
}

