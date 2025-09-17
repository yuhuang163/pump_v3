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

#ifndef __BT_TESTER_CMD_H__
#define __BT_TESTER_CMD_H__

#include "bt_drv_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN  (0x4B)
#define BT_TESTER_CMD_BLE_TX_V3_CMD_MAX_LEN  (11 + BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN)
#define BT_TESTER_CMD_BLE_RX_V3_CMD_MAX_LEN  (11 + BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN)
#define BT_TESTER_CMD_BLE_TX_V4_CMD_MAX_LEN  (12 + BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN)

enum BT_TESTER_SCAN_TYPE_T {
    BT_TESTER_BOTH_SCAN_DISABLE = 0,
    BT_TESTER_INQUIRY_SCAN_ENABLE = 1,
    BT_TESTER_PAGE_SCAN_ENABLE = 2,
    BT_TESTER_BOTH_SCAN_ENABLE = 3,
};

typedef struct
{
    uint8_t channel;
    uint8_t test_data_len;
    uint8_t pkt_payload;
    uint8_t phy;
    uint8_t cte_len;
    uint8_t cte_type;
    uint8_t switching_pattern_len;
    uint8_t antenna_id[BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN];
}bt_tester_cmd_le_tx_test_v3_t;

typedef struct
{
    uint8_t channel;
    uint8_t test_data_len;
    uint8_t pkt_payload;
    uint8_t phy;
    uint8_t cte_len;
    uint8_t cte_type;
    uint8_t switching_pattern_len;
    uint8_t antenna_id[BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN];
    uint8_t power_level;
}bt_tester_cmd_le_tx_test_v4_t;

typedef struct
{
    uint8_t channel;
    uint8_t phy;
    uint8_t modulation_idx;
    uint8_t exp_cte_len;
    uint8_t exp_cte_type;
    uint8_t slot_durations;
    uint8_t switching_pattern_len;
    uint8_t antenna_id[BT_TESTER_CMD_MAX_SWITCHING_PATTERN_LEN];
}bt_tester_cmd_le_rx_test_v3_t;

void bt_tester_cmd_hci_reset(void);
void bt_tester_cmd_wr_bt_addr(uint8_t* addr);
void bt_tester_cmd_rd_bt_addr(void);
void bt_tester_cmd_wr_ble_pubaddr(uint8_t* addr);
void bt_tester_cmd_rd_ble_pubaddr(void);
void bt_tester_cmd_wr_ble_randaddr(uint8_t *addr);
void bt_tester_cmd_wr_bt_local_feat(void);
void bt_tester_cmd_rd_bt_local_feat(void);
void bt_tester_cmd_wr_pscan_activity(uint16_t pscan_interval, uint16_t pscan_window);
void bt_tester_cmd_rd_pscan_activity(void);
void bt_tester_cmd_wr_iscan_activity(uint16_t iscan_interval, uint16_t iscan_window);
void bt_tester_cmd_rd_iscan_activity(void);
void bt_tester_cmd_scan_control(enum BT_TESTER_SCAN_TYPE_T type);
void bt_tester_cmd_set_bt_sleep(uint8_t enable);
void bt_tester_cmd_set_bt_power(struct btdrv_customer_rf_config_t* config);
void bt_tester_cmd_set_ble_power(struct btdrv_customer_rf_config_t* config);
void bt_tester_cmd_rd_bt_power(void);
void bt_tester_cmd_rd_ble_power(void);
void bt_tester_cmd_le_tx_test_v1(uint8_t channel, uint8_t test_data_len, uint8_t pkt_payload);
void bt_tester_cmd_le_tx_test_v2(uint8_t channel, uint8_t package_len, uint8_t payload_type, uint8_t phy);
void bt_tester_cmd_le_tx_test_v3(bt_tester_cmd_le_tx_test_v3_t param);
void bt_tester_cmd_le_tx_test_v4(bt_tester_cmd_le_tx_test_v4_t param);
void bt_tester_cmd_le_rx_test_v1(uint8_t channel);
void bt_tester_cmd_le_rx_test_v2(uint8_t channel, uint8_t phy, uint8_t modulation_index);
void bt_tester_cmd_le_rx_test_v3(bt_tester_cmd_le_rx_test_v3_t param);
void bt_tester_cmd_le_test_end(void);
void bt_tester_cmd_nosig_bdr_tx_test(uint8_t channel, uint8_t package_type, uint8_t payload_type, uint8_t power_level, uint8_t *addr);
void bt_tester_cmd_nosig_edr_tx_test(uint8_t channel, uint8_t package_type, uint8_t payload_type, uint8_t power_level, uint8_t *addr);
void bt_tester_cmd_nosig_bdr_rx_test(uint8_t channel, uint8_t package_type, uint8_t payload_type, uint8_t power_level, uint8_t *addr);
void bt_tester_cmd_nosig_edr_rx_test(uint8_t channel, uint8_t package_type, uint8_t payload_type, uint8_t power_level, uint8_t *addr);
void bt_tester_cmd_nosig_stop_test(void);
void bt_tester_cmd_enter_signal_mode(void);
void bt_tester_cmd_carrier_start(uint8_t channel);
void bt_tester_cmd_carrier_stop(void);

#ifdef __cplusplus
}
#endif
#endif
