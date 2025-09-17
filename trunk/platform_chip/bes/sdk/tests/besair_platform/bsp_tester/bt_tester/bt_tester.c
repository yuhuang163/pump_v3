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
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "besbt_string.h"
#include "bt_tester.h"
#include "bt_drv_internal.h"
#include "bt_drv_interface.h"
#include "bt_tester_cmd.h"
#include "bt_drv.h"
#include "bt_drv_reg_op.h"
#include "common.h"
#include "bt_drv_common_test.h"
#include "bt_drv_common_test_case.h"


#define CMD_MAX_ARGC    30

enum bt_test_cmd_e {
    CMD_BT_RESET = 0,
    CMD_WR_BT_ADDR = 1,
    CMD_RD_BT_ADDR = 2,
    CMD_WR_BLE_PUBADDR = 3,
    CMD_RD_BLE_PUBADDR = 4,
    CMD_WR_BLE_RANDADDR = 5,
    CMD_WR_BT_LOCAL_FEAT = 6,
    CMD_RD_BT_LOCAL_FEAT = 7,
    CMD_WR_PSCAN_ACTIVITY = 8,
    CMD_RD_PSCAN_ACTIVITY = 9,
    CMD_WR_ISCAN_ACTIVITY = 10,
    CMD_RD_ISCAN_ACTIVITY = 11,
    CMD_BT_SCAN_CONTROL = 12,
    CMD_SET_BT_SLEEP = 13,
    CMD_SET_BT_POWER = 14,
    CMD_SET_BLE_POWER = 15,
    CMD_RD_BT_POWER = 16,
    CMD_RD_BLE_POWER = 17,
    CMD_BLE_TX_TEST_V1 = 18,
    CMD_BLE_TX_TEST_V2 = 19,
    CMD_BLE_TX_TEST_V3 = 20,
    CMD_BLE_TX_TEST_V4 = 21,
    CMD_BLE_RX_TEST_V1 = 22,
    CMD_BLE_RX_TEST_V2 = 23,
    CMD_BLE_RX_TEST_V3 = 24,
    CMD_BLE_TEST_END = 25,
    CMD_BT_NOSIG_BDR_TX = 26,
    CMD_BT_NOSIG_BDR_RX = 27,
    CMD_BT_NOSIG_EDR_TX = 28,
    CMD_BT_NOSIG_EDR_RX = 29,
    CMD_BT_NOSIG_STOP_TEST = 30,
    CMD_ENTER_SIG_MODE = 31,
    CMD_BT_CARRIER_START = 32,
    CMD_BT_CARRIER_STOP = 33,
    CMD_PSCAN_STANDARD_ENABLE = 34,
    CMD_PSCAN_INTERLACED_ENABLE = 35,
    CMD_BOTHSCAN_ENABLE = 36,
    CMD_BT_CREATE_CONNECTION = 37,
    CMD_BT_ENTER_SNIFF = 38,
    CMD_BLE_ADV_ENABLE = 39,
    CMD_BLE_CREATE_CONNECTION = 40,
    CMD_BLE_ADV_ENABLE_WITH_DATA = 41,
    CMD_MAX,
};

typedef struct {
    int index;
    char const* cmd;
} bt_test_cmd_t;

typedef struct {
    const char* type;
    uint8_t idex;
} bt_test_type_t;

static bt_test_cmd_t bt_test_cmd[] = {
    { CMD_BT_RESET, "HCI_RESET" },
    { CMD_WR_BT_ADDR, "WR_BTADDR" },
    { CMD_RD_BT_ADDR, "RD_BTADDR" },
    { CMD_WR_BLE_PUBADDR, "WR_BLE_PUBADDR" },
    { CMD_RD_BLE_PUBADDR, "RD_BLE_PUBADDR" },
    { CMD_WR_BLE_RANDADDR, "WR_BLE_RANDADDR" },
    { CMD_WR_BT_LOCAL_FEAT, "WR_BT_LOCAL_FEAT" },
    { CMD_RD_BT_LOCAL_FEAT, "RD_BT_LOCAL_FEAT" },
    { CMD_WR_PSCAN_ACTIVITY, "WR_PSCAN_ACT" },
    { CMD_RD_PSCAN_ACTIVITY, "RD_PSCAN_ACT" },
    { CMD_WR_ISCAN_ACTIVITY, "WR_ISCAN_ACT" },
    { CMD_RD_ISCAN_ACTIVITY, "RD_ISCAN_ACT" },
    { CMD_BT_SCAN_CONTROL, "BT_SCAN_CONTROL" },
    { CMD_SET_BT_SLEEP, "SET_BT_SLEEP" },
    { CMD_SET_BT_POWER, "SET_BT_POWER" },
    { CMD_SET_BLE_POWER, "SET_BLE_POWER" },
    { CMD_RD_BT_POWER, "RD_BT_POWER" },
    { CMD_RD_BLE_POWER, "RD_BLE_POWER" },
    { CMD_BLE_TX_TEST_V1, "BLE_TX_TEST_V1" },
    { CMD_BLE_TX_TEST_V2, "BLE_TX_TEST_V2" },
    { CMD_BLE_TX_TEST_V3, "BLE_TX_TEST_V3" },
    { CMD_BLE_TX_TEST_V4, "BLE_TX_TEST_V4" },
    { CMD_BLE_RX_TEST_V1, "BLE_RX_TEST_V1" },
    { CMD_BLE_RX_TEST_V2, "BLE_RX_TEST_V2" },
    { CMD_BLE_RX_TEST_V3, "BLE_RX_TEST_V3" },
    { CMD_BLE_TEST_END, "BLE_TEST_END" },
    { CMD_BT_NOSIG_BDR_TX, "NOSIG_BDR_TX" },
    { CMD_BT_NOSIG_BDR_RX, "NOSIG_BDR_RX" },
    { CMD_BT_NOSIG_EDR_TX, "NOSIG_EDR_TX" },
    { CMD_BT_NOSIG_EDR_RX, "NOSIG_EDR_RX" },
    { CMD_BT_NOSIG_STOP_TEST, "NOSIG_STOP" },
    { CMD_ENTER_SIG_MODE, "SIG_MODE" },
    { CMD_BT_CARRIER_START, "BT_CARRIER_START" },
    { CMD_BT_CARRIER_STOP, "BT_CARRIER_STOP" },
    { CMD_PSCAN_STANDARD_ENABLE, "PSCAN_STANDARD" },
    { CMD_PSCAN_INTERLACED_ENABLE, "PSCAN_INTERLACED" },
    { CMD_BOTHSCAN_ENABLE, "BOTHSCAN" },
    { CMD_BT_CREATE_CONNECTION, "BT_CREATE_CONNECTION" },
    { CMD_BT_ENTER_SNIFF, "BT_ENTER_SNIFF" },
    { CMD_BLE_ADV_ENABLE, "BLE_ADV" },
    { CMD_BLE_CREATE_CONNECTION, "BLE_CREATE_CONNECTION" },
    { CMD_BLE_ADV_ENABLE_WITH_DATA, "BLE_ADV_WITH_DATA" },
};

static const bt_test_type_t bt_packet_type[] = {
    { "DH1", 0x4 },
    { "2DH1", 0x4 },
    { "3DH1", 0x8 },
    { "DH3", 0xb },
    { "2DH3", 0xa },
    { "3DH3", 0xb },
    { "DH5", 0xf },
    { "2DH5", 0xe },
    { "3DH5", 0xf },
};

static const bt_test_type_t bt_payload_type[] = {
    { "0x00", 0x0 },
    { "0xFF", 0x1 },
    { "0x55", 0x2 },
    { "0x0F", 0x3 },
    { "PRBS9", 0x4 },
};

static const bt_test_type_t ble_payload_type[] = {
    { "PRBS9", 0x0 },
    { "0x0F", 0x1 },
    { "0x55", 0x2 },
    { "PRBS15", 0x3 },
    { "0xFF", 0x4 },
    { "0x00", 0x5 },
    { "0xF0", 0x6 },
    { "0xAA", 0x7 },
};

static const bt_test_type_t ble_phy_type[] = {
    { "LE_1M", 0x1 },
    { "LE_2M", 0x2 },
    { "LE_CODED_S8", 0x3 },
    { "LE_CODED_S2", 0x4 },
};

static const bt_test_type_t ble_modulation_type[] = {
    { "STANDARD", 0x0 },
    { "STABLE", 0x1 },
};

static const bt_test_type_t ble_cte_type[] = {
    { "CTE_TYPE_AOA", 0x0 },
    { "CTE_TYPE_AOD_1US", 0x1 },
    { "CTE_TYPE_AOD_2US", 0x2 },
};

static const bt_test_type_t ble_slot_duration_type[] = {
    { "SLOT_DUR_1US", 0x1 },
    { "SLOT_DUR_2US", 0x2 },
};

static int bt_test_cmd_get(const char* cmd)
{
    for (int i = 0; i < CMD_MAX; i++) {
        if (strncmp(bt_test_cmd[i].cmd, cmd, strlen(cmd)) == 0) {
            return bt_test_cmd[i].index;
        }
    }
    return CMD_MAX;
}

static uint8_t bt_packet_type_get(char const* type)
{
    for (int i = 0; i < (sizeof(bt_packet_type) / sizeof(bt_test_type_t)); i++) {
        if (strncmp(type, bt_packet_type[i].type, strlen(type)) == 0) {
            return bt_packet_type[i].idex;
        }
    }
    return 0xff;
}

static uint8_t bt_payload_type_get(char const* type)
{
    for (int i = 0; i < (sizeof(bt_payload_type) / sizeof(bt_test_type_t)); i++) {
        if (strncmp(type, bt_payload_type[i].type, strlen(type)) == 0) {
            return bt_payload_type[i].idex;
        }
    }
    return 0xff;
}

static uint8_t ble_payload_type_get(char const* type)
{
    for (int i = 0; i < (sizeof(ble_payload_type) / sizeof(bt_test_type_t)); i++) {
        if (strncmp(type, ble_payload_type[i].type, strlen(type)) == 0) {
            return ble_payload_type[i].idex;
        }
    }
    return 0xff;
}

static uint8_t ble_phy_type_get(char const* type)
{
    for (int i = 0; i < (sizeof(ble_phy_type) / sizeof(bt_test_type_t)); i++) {
        if (strncmp(type, ble_phy_type[i].type, strlen(type)) == 0) {
            return ble_phy_type[i].idex;
        }
    }
    return 0xff;
}

static uint8_t ble_modulation_type_get(char const* type)
{
    for (int i = 0; i < (sizeof(ble_modulation_type) / sizeof(bt_test_type_t)); i++) {
        if (strncmp(type, ble_modulation_type[i].type, strlen(type)) == 0) {
            return ble_modulation_type[i].idex;
        }
    }
    return 0xff;
}

static uint8_t ble_cte_type_get(char const* type)
{
    for (int i = 0; i < (sizeof(ble_cte_type) / sizeof(bt_test_type_t)); i++) {
        if (strncmp(type, ble_cte_type[i].type, strlen(type)) == 0) {
            return ble_cte_type[i].idex;
        }
    }
    return 0xff;
}

static uint8_t ble_slot_duration_type_get(char const* type)
{
    for (int i = 0; i < (sizeof(ble_slot_duration_type) / sizeof(bt_test_type_t)); i++) {
        if (strncmp(type, ble_slot_duration_type[i].type, strlen(type)) == 0) {
            return ble_slot_duration_type[i].idex;
        }
    }
    return 0xff;
}

void bt_test_cmd_send(const char** params)
{
    int cmd_index = bt_test_cmd_get(params[0]);

    CMD_TRACE("cmd_index = %d",cmd_index);
    switch (cmd_index)
    {
        case CMD_BT_RESET:
        {
            bt_tester_cmd_hci_reset();
            break;
        }
        case CMD_WR_BT_ADDR:
        {
            uint8_t test_addr[9] = { 0 };
            sscanf(params[1], "%02x%02x%02x%02x%02x%02x",
                (uint32_t *)(&test_addr[0]), (uint32_t *)(&test_addr[1]),
                (uint32_t *)(&test_addr[2]), (uint32_t *)(&test_addr[3]),
                (uint32_t *)(&test_addr[4]), (uint32_t *)(&test_addr[5]));
            bt_tester_cmd_wr_bt_addr(test_addr);
            break;
        }
        case CMD_RD_BT_ADDR:
        {
            bt_tester_cmd_rd_bt_addr();
            break;
        }
        case CMD_WR_BLE_PUBADDR:
        {
            uint8_t test_addr[9] = { 0 };
            sscanf(params[1], "%02x%02x%02x%02x%02x%02x",
                (uint32_t *)(&test_addr[0]), (uint32_t *)(&test_addr[1]),
                (uint32_t *)(&test_addr[2]), (uint32_t *)(&test_addr[3]),
                (uint32_t *)(&test_addr[4]), (uint32_t *)(&test_addr[5]));
            bt_tester_cmd_wr_ble_pubaddr(test_addr);
            break;
        }
        case CMD_RD_BLE_PUBADDR:
        {
            bt_tester_cmd_rd_ble_pubaddr();
            break;
        }
        case CMD_WR_BLE_RANDADDR:
        {
            uint8_t test_addr[9] = { 0 };
            sscanf(params[1], "%02x%02x%02x%02x%02x%02x",
                (uint32_t *)(&test_addr[0]), (uint32_t *)(&test_addr[1]),
                (uint32_t *)(&test_addr[2]), (uint32_t *)(&test_addr[3]),
                (uint32_t *)(&test_addr[4]), (uint32_t *)(&test_addr[5]));
            bt_tester_cmd_wr_ble_randaddr(test_addr);
            break;
        }
        case CMD_WR_BT_LOCAL_FEAT:
        {
            bt_tester_cmd_wr_bt_local_feat();
            break;
        }
        case CMD_RD_BT_LOCAL_FEAT:
        {
            bt_tester_cmd_rd_bt_local_feat();
            break;
        }
        case CMD_WR_PSCAN_ACTIVITY:
        {
            uint16_t pscan_interval = 0;
            uint16_t pscan_window = 0;
            uint8_t temp[7] = { 0 };
            sscanf(params[1], "%02x%02x%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]),
                (uint32_t *)(&temp[2]), (uint32_t *)(&temp[3]));
            pscan_interval = (uint16_t)(temp[0] << 8) | temp[1];
            pscan_window = (uint16_t)(temp[2] << 8) | temp[3];
            bt_tester_cmd_wr_pscan_activity(pscan_interval,pscan_window);
            break;
        }
        case CMD_RD_PSCAN_ACTIVITY:
        {
            bt_tester_cmd_rd_pscan_activity();
            break;
        }
        case CMD_WR_ISCAN_ACTIVITY:
        {
            uint16_t iscan_interval = 0;
            uint16_t iscan_window = 0;
            uint8_t temp[7] = { 0 };
            sscanf(params[1], "%02x%02x%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]),
                (uint32_t *)(&temp[2]), (uint32_t *)(&temp[3]));
            iscan_interval = (uint16_t)(temp[0] << 8) | temp[1];
            iscan_window = (uint16_t)(temp[2] << 8) | temp[3];
            bt_tester_cmd_wr_iscan_activity(iscan_interval,iscan_window);
            break;
        }
        case CMD_RD_ISCAN_ACTIVITY:
        {
            bt_tester_cmd_rd_iscan_activity();
            break;
        }
        case CMD_BT_SCAN_CONTROL:
        {
            uint8_t scan_type = atoi(params[1]);
            bt_tester_cmd_scan_control(scan_type);
            break;
        }
        case CMD_SET_BT_SLEEP:
        {
            uint8_t enable = atoi(params[1]);
            bt_tester_cmd_set_bt_sleep(enable);
            break;
        }
        case CMD_SET_BT_POWER:
        {
            bt_tester_cmd_set_bt_power(NULL);
            break;
        }
        case CMD_SET_BLE_POWER:
        {
            bt_tester_cmd_set_ble_power(NULL);
            break;
        }
        case CMD_RD_BT_POWER:
        {
            bt_tester_cmd_rd_bt_power();
            break;
        }
        case CMD_RD_BLE_POWER:
        {
            bt_tester_cmd_rd_ble_power();
            break;
        }
        case CMD_BLE_TX_TEST_V1:
        {
            uint8_t channel = atoi(params[1]);
            uint8_t test_data_len = atoi(params[2]);
            uint8_t pkt_payload = ble_payload_type_get(params[3]);
            bt_tester_cmd_le_tx_test_v1(channel, test_data_len, pkt_payload);
            break;
        }
        case CMD_BLE_TX_TEST_V2:
        {
            uint8_t channel = atoi(params[1]);
            uint8_t test_data_len = atoi(params[2]);
            uint8_t pkt_payload = ble_payload_type_get(params[3]);
            uint8_t phy_type = ble_phy_type_get(params[4]);
            bt_tester_cmd_le_tx_test_v2(channel, test_data_len, pkt_payload, phy_type);
            break;
        }
        case CMD_BLE_TX_TEST_V3:
        {
            uint8_t temp[BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN] = {0};
            bt_tester_cmd_le_tx_test_v3_t param;
            param.channel = atoi(params[1]);
            param.test_data_len = atoi(params[2]);
            param.pkt_payload = ble_payload_type_get(params[3]);
            param.phy = ble_phy_type_get(params[4]);
            param.cte_len = atoi(params[5]);
            param.cte_type = ble_cte_type_get(params[6]);
            param.switching_pattern_len = atoi(params[7]);
            for(uint8_t i = 0;i < param.switching_pattern_len;i++)
            {
                temp[i] = atoi(params[8+i]);
            }
            memcpy(&param.antenna_id[0], &temp[0], param.switching_pattern_len);
            bt_tester_cmd_le_tx_test_v3(param);
            break;
        }
        case CMD_BLE_TX_TEST_V4:
        {
            uint8_t temp[BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN] = {0};
            bt_tester_cmd_le_tx_test_v4_t param;
            param.channel = atoi(params[1]);
            param.test_data_len = atoi(params[2]);
            param.pkt_payload = ble_payload_type_get(params[3]);
            param.phy = ble_phy_type_get(params[4]);
            param.cte_len = atoi(params[5]);
            param.cte_type = ble_cte_type_get(params[6]);
            param.switching_pattern_len = atoi(params[7]);
            for(uint8_t i = 0;i < param.switching_pattern_len;i++)
            {
                temp[i] = atoi(params[8+i]);
            }
            memcpy(&param.antenna_id[0], &temp[0], param.switching_pattern_len);
            param.power_level = atoi(params[8+param.switching_pattern_len]);
            bt_tester_cmd_le_tx_test_v4(param);
            break;
        }
        case CMD_BLE_RX_TEST_V1:
        {
            uint8_t channel = atoi(params[1]);
            bt_tester_cmd_le_rx_test_v1(channel);
            break;
        }
        case CMD_BLE_RX_TEST_V2:
        {
            uint8_t channel = atoi(params[1]);
            uint8_t phy_type = ble_phy_type_get(params[2]);
            uint8_t modulation = ble_modulation_type_get(params[3]);
            bt_tester_cmd_le_rx_test_v2(channel, phy_type, modulation);
            break;
        }
        case CMD_BLE_RX_TEST_V3:
        {
            uint8_t temp[BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN] = {0};
            bt_tester_cmd_le_rx_test_v3_t param;
            param.channel = atoi(params[1]);
            param.phy = ble_phy_type_get(params[2]);
            param.modulation_idx = ble_modulation_type_get(params[3]);
            param.exp_cte_len = atoi(params[4]);
            param.exp_cte_type = ble_cte_type_get(params[5]);
            param.slot_durations = ble_slot_duration_type_get(params[6]);
            param.switching_pattern_len = atoi(params[7]);
            for(uint8_t i = 0;i < param.switching_pattern_len;i++)
            {
                temp[i] = atoi(params[8+i]);
            }
            memcpy(&param.antenna_id[0], &temp[0], param.switching_pattern_len);
            bt_tester_cmd_le_rx_test_v3(param);
            break;
        }
        case CMD_BLE_TEST_END:
        {
            bt_tester_cmd_le_test_end();
            break;
        }
        case CMD_BT_NOSIG_BDR_TX:
        {
            uint8_t channel = atoi(params[1]);
            uint8_t package_type = bt_packet_type_get(params[2]);
            uint8_t payload_type = bt_payload_type_get(params[3]);
            uint8_t power_level = atoi(params[4]);
            uint8_t test_addr[9] = { 0 };
            sscanf(params[5], "%02x%02x%02x%02x%02x%02x",
                (uint32_t *)(&test_addr[0]), (uint32_t *)(&test_addr[1]),
                (uint32_t *)(&test_addr[2]), (uint32_t *)(&test_addr[3]),
                (uint32_t *)(&test_addr[4]), (uint32_t *)(&test_addr[5]));
            bt_tester_cmd_nosig_bdr_tx_test(channel, package_type, payload_type, power_level, test_addr);
            break;
        }
        case CMD_BT_NOSIG_EDR_TX:
        {
            uint8_t channel = atoi(params[1]);
            uint8_t package_type = bt_packet_type_get(params[2]);
            uint8_t payload_type = bt_payload_type_get(params[3]);
            uint8_t power_level = atoi(params[4]);
            uint8_t test_addr[9] = { 0 };
            sscanf(params[5], "%02x%02x%02x%02x%02x%02x",
                (uint32_t *)(&test_addr[0]), (uint32_t *)(&test_addr[1]),
                (uint32_t *)(&test_addr[2]), (uint32_t *)(&test_addr[3]),
                (uint32_t *)(&test_addr[4]), (uint32_t *)(&test_addr[5]));
            bt_tester_cmd_nosig_edr_tx_test(channel, package_type, payload_type, power_level, test_addr);
            break;
        }
        case CMD_BT_NOSIG_BDR_RX:
        {
            uint8_t channel = atoi(params[1]);
            uint8_t package_type = bt_packet_type_get(params[2]);
            uint8_t payload_type = bt_payload_type_get(params[3]);
            uint8_t power_level = atoi(params[4]);
            uint8_t test_addr[9] = { 0 };
            sscanf(params[5], "%02x%02x%02x%02x%02x%02x",
                (uint32_t *)(&test_addr[0]), (uint32_t *)(&test_addr[1]),
                (uint32_t *)(&test_addr[2]), (uint32_t *)(&test_addr[3]),
                (uint32_t *)(&test_addr[4]), (uint32_t *)(&test_addr[5]));
            bt_tester_cmd_nosig_bdr_rx_test(channel, package_type, payload_type, power_level, test_addr);
            break;
        }
        case CMD_BT_NOSIG_EDR_RX:
        {
            uint8_t channel = atoi(params[1]);
            uint8_t package_type = bt_packet_type_get(params[2]);
            uint8_t payload_type = bt_payload_type_get(params[3]);
            uint8_t power_level = atoi(params[4]);
            uint8_t test_addr[9] = { 0 };
            sscanf(params[5], "%02x%02x%02x%02x%02x%02x",
                (uint32_t *)(&test_addr[0]), (uint32_t *)(&test_addr[1]),
                (uint32_t *)(&test_addr[2]), (uint32_t *)(&test_addr[3]),
                (uint32_t *)(&test_addr[4]), (uint32_t *)(&test_addr[5]));
            bt_tester_cmd_nosig_edr_rx_test(channel, package_type, payload_type, power_level, test_addr);
            break;
        }
        case CMD_BT_NOSIG_STOP_TEST:
        {
            bt_tester_cmd_nosig_stop_test();
            break;
        }
        case CMD_ENTER_SIG_MODE:
        {
            bt_tester_cmd_enter_signal_mode();
            break;
        }
        case CMD_BT_CARRIER_START:
        {
            uint8_t channel = atoi(params[1]);
            bt_tester_cmd_carrier_start(channel);
            break;
        }
        case CMD_BT_CARRIER_STOP:
        {
            bt_tester_cmd_carrier_stop();
            break;
        }
        case CMD_PSCAN_STANDARD_ENABLE:
        {
            uint16_t pscan_interval = 0;
            uint16_t pscan_window = 0;
            uint8_t temp[7] = { 0 };
            sscanf(params[1], "%02x%02x%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]),
                (uint32_t *)(&temp[2]), (uint32_t *)(&temp[3]));
            pscan_interval = (uint16_t)(temp[0] << 8) | temp[1];
            pscan_window = (uint16_t)(temp[2] << 8) | temp[3];
            btdrv_common_pscan_standard_enable_test(pscan_interval,pscan_window);
            break;
        }
        case CMD_PSCAN_INTERLACED_ENABLE:
        {
            uint16_t pscan_interval = 0;
            uint16_t pscan_window = 0;
            uint8_t temp[7] = { 0 };
            sscanf(params[1], "%02x%02x%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]),
                (uint32_t *)(&temp[2]), (uint32_t *)(&temp[3]));
            pscan_interval = (uint16_t)(temp[0] << 8) | temp[1];
            pscan_window = (uint16_t)(temp[2] << 8) | temp[3];
            btdrv_common_pscan_interlaced_enable_test(pscan_interval,pscan_window);
            break;
        }
        case CMD_BOTHSCAN_ENABLE:
        {
            uint16_t pscan_interval = 0;
            uint16_t pscan_window = 0;
            uint8_t temp[7] = { 0 };
            sscanf(params[1], "%02x%02x%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]),
                (uint32_t *)(&temp[2]), (uint32_t *)(&temp[3]));
            pscan_interval = (uint16_t)(temp[0] << 8) | temp[1];
            pscan_window = (uint16_t)(temp[2] << 8) | temp[3];
            btdrv_common_bothscan_enable_test(pscan_interval,pscan_window);
            break;
        }
        case CMD_BT_CREATE_CONNECTION:
        {
            btdrv_common_bt_create_connect_test();
            break;
        }
        case CMD_BT_ENTER_SNIFF:
        {
            uint16_t conhandle = 0;
            uint16_t sniff_interval = 0;
            uint16_t attempt = 0;
            uint16_t timeout = 0;
            uint8_t temp[11] = { 0 };
            sscanf(params[1], "%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]));
            conhandle = (uint16_t)(temp[0] << 8) | temp[1];
            sscanf(params[2], "%02x%02x",
                (uint32_t *)(&temp[2]), (uint32_t *)(&temp[3]));
            sniff_interval = (uint16_t)(temp[2] << 8) | temp[3];
            sscanf(params[3], "%02x%02x",
                (uint32_t *)(&temp[4]), (uint32_t *)(&temp[5]));
            attempt = (uint16_t)(temp[4] << 8) | temp[5];
            sscanf(params[4], "%02x%02x",
                (uint32_t *)(&temp[6]), (uint32_t *)(&temp[7]));
            timeout = (uint16_t)(temp[6] << 8) | temp[7];
            btdrv_common_bt_enter_sniff_test(conhandle,sniff_interval,attempt,timeout);
            break;
        }
        case CMD_BLE_ADV_ENABLE:
        {
            uint16_t adv_interval = 0;
            uint8_t temp[7] = { 0 };
            sscanf(params[1], "%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]));
            adv_interval = (uint16_t)(temp[0] << 8) | temp[1];
            btdrv_common_ble_adv_enable_test(adv_interval);
            break;
        }
        case CMD_BLE_CREATE_CONNECTION:
        {
            uint16_t conn_interval = 0;
            uint8_t temp[7] = { 0 };
            sscanf(params[1], "%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]));
            conn_interval = (uint16_t)(temp[0] << 8) | temp[1];
            btdrv_common_ble_create_connect_test(conn_interval);
            break;
        }
        case CMD_BLE_ADV_ENABLE_WITH_DATA:
        {
            uint16_t adv_interval = 0;
            uint8_t temp[7] = { 0 };
            sscanf(params[1], "%02x%02x",
                (uint32_t *)(&temp[0]), (uint32_t *)(&temp[1]));
            adv_interval = (uint16_t)(temp[0] << 8) | temp[1];
            btdrv_common_ble_adv_enable_with_data_test(adv_interval);
            break;
        }
        default:
        {
            CMD_TRACE("NO CMD FOUND");
            break;
        }
    }
}

static void bt_tester_bt_init(void)
{
    btdrv_hciopen();
    btdrv_ins_patch_test_init();
    bt_drv_reg_op_key_gen_after_reset(false);
    btdrv_hci_reset();

    btdrv_sleep_config(0);
    osDelay(2000);
    btdrv_testmode_start();
    btdrv_feature_default();
    bt_drv_config_after_hci_reset();
}

void bt_tester(const char **params)
{
    static bool entry = false;

    if (!entry)
    {
        /* bluetooth initialization */
        btdrv_start_bt();
        bt_tester_bt_init();
        entry = true;
    }

    bt_test_cmd_send(params);

}


