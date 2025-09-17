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

#ifndef __BT_DRV_COMMON_TEST_CASE_H__
#define __BT_DRV_COMMON_TEST_CASE_H__

#include "bt_drv_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BTDRV_COMMON_TEST_CMD_MAX_SWITCHING_PATTERN_LEN  (0x4B)
#define BTDRV_COMMON_TEST_CMD_BLE_TX_V3_CMD_MAX_LEN  (11 + BTDRV_COMMON_TEST_CMD_MAX_SWITCHING_PATTERN_LEN)
#define BTDRV_COMMON_TEST_CMD_BLE_RX_V3_CMD_MAX_LEN  (11 + BTDRV_COMMON_TEST_CMD_MAX_SWITCHING_PATTERN_LEN)
#define BTDRV_COMMON_TEST_CMD_BLE_TX_V4_CMD_MAX_LEN  (12 + BTDRV_COMMON_TEST_CMD_MAX_SWITCHING_PATTERN_LEN)

enum BTDRV_COMMON_TEST_SCAN_TYPE_T {
    BTDRV_COMMON_TEST_BOTH_SCAN_DISABLE = 0,
    BTDRV_COMMON_TEST_INQUIRY_SCAN_ENABLE = 1,
    BTDRV_COMMON_TEST_PAGE_SCAN_ENABLE = 2,
    BTDRV_COMMON_TEST_BOTH_SCAN_ENABLE = 3,
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
    uint8_t antenna_id[BTDRV_COMMON_TEST_CMD_MAX_SWITCHING_PATTERN_LEN];
}btdrv_common_test_cmd_le_tx_test_v3_t;

typedef struct
{
    uint8_t channel;
    uint8_t test_data_len;
    uint8_t pkt_payload;
    uint8_t phy;
    uint8_t cte_len;
    uint8_t cte_type;
    uint8_t switching_pattern_len;
    uint8_t antenna_id[BTDRV_COMMON_TEST_CMD_MAX_SWITCHING_PATTERN_LEN];
    uint8_t power_level;
}btdrv_common_test_cmd_le_tx_test_v4_t;

typedef struct
{
    uint8_t channel;
    uint8_t phy;
    uint8_t modulation_idx;
    uint8_t exp_cte_len;
    uint8_t exp_cte_type;
    uint8_t slot_durations;
    uint8_t switching_pattern_len;
    uint8_t antenna_id[BTDRV_COMMON_TEST_CMD_MAX_SWITCHING_PATTERN_LEN];
}btdrv_common_test_cmd_le_rx_test_v3_t;

void btdrv_common_test_cmd_hci_reset(void);
void btdrv_common_test_cmd_wr_bt_addr(uint8_t* addr);
void btdrv_common_test_cmd_rd_bt_addr(void);
void btdrv_common_test_cmd_wr_ble_pubaddr(uint8_t* addr);
void btdrv_common_test_cmd_rd_ble_pubaddr(void);
void btdrv_common_test_cmd_wr_ble_randaddr(uint8_t *addr);
void btdrv_common_test_cmd_wr_bt_local_feat(void);
void btdrv_common_test_cmd_rd_bt_local_feat(void);
void btdrv_common_test_cmd_wr_pscan_activity(uint16_t pscan_interval, uint16_t pscan_window);
void btdrv_common_test_cmd_rd_pscan_activity(void);
void btdrv_common_test_cmd_wr_iscan_activity(uint16_t iscan_interval, uint16_t iscan_window);
void btdrv_common_test_cmd_rd_iscan_activity(void);
void btdrv_common_test_cmd_scan_control(enum BTDRV_COMMON_TEST_SCAN_TYPE_T type);
void btdrv_common_test_cmd_wr_pagescan_type(uint8_t pscan_type);
void btdrv_common_test_cmd_set_bt_sleep(uint8_t enable);
void btdrv_common_test_cmd_bt_ble_tx_power_fix(int8_t bt_tx_pwr_fix_dbm,int8_t ble_tx_pwr_fix_dbm);
void btdrv_common_test_cmd_rd_bt_power(uint16_t conhandle);
void btdrv_common_test_cmd_rd_ble_power(uint16_t conhandle);
void btdrv_common_test_cmd_enable_bt_ble_rx_gain_fix(uint8_t enable,uint8_t bt_ble_mode,uint8_t gain_idx);
void btdrv_common_test_cmd_le_tx_test_v1(uint8_t channel, uint8_t test_data_len, uint8_t pkt_payload);
void btdrv_common_test_cmd_le_tx_test_v2(uint8_t channel, uint8_t package_len, uint8_t payload_type, uint8_t phy);
void btdrv_common_test_cmd_le_tx_test_v3(btdrv_common_test_cmd_le_tx_test_v3_t param);
void btdrv_common_test_cmd_le_tx_test_v4(btdrv_common_test_cmd_le_tx_test_v4_t param);
void btdrv_common_test_cmd_le_rx_test_v1(uint8_t channel);
void btdrv_common_test_cmd_le_rx_test_v2(uint8_t channel, uint8_t phy, uint8_t modulation_index);
void btdrv_common_test_cmd_le_rx_test_v3(btdrv_common_test_cmd_le_rx_test_v3_t param);
void btdrv_common_test_cmd_le_test_end(void);
void btdrv_common_test_cmd_nosig_bdr_tx_test(uint8_t channel, uint8_t package_type, uint8_t payload_type, uint8_t power_level, uint8_t *addr);
void btdrv_common_test_cmd_nosig_edr_tx_test(uint8_t channel, uint8_t package_type, uint8_t payload_type, uint8_t power_level, uint8_t *addr);
void btdrv_common_test_cmd_nosig_bdr_rx_test(uint8_t channel, uint8_t package_type, uint8_t payload_type, uint8_t power_level, uint8_t *addr);
void btdrv_common_test_cmd_nosig_edr_rx_test(uint8_t channel, uint8_t package_type, uint8_t payload_type, uint8_t power_level, uint8_t *addr);
void btdrv_common_test_cmd_nosig_stop_test(void);
void btdrv_common_test_cmd_enter_signal_mode(void);
void btdrv_common_test_cmd_carrier_start(uint8_t channel);
void btdrv_common_test_cmd_carrier_stop(void);
void btdrv_common_test_cmd_ble_set_adv_param(uint16_t adv_interval_min,uint16_t adv_interval_max,uint8_t adv_type,
                                                    uint8_t own_addr_type,uint8_t peer_addr_type,uint8_t *peer_addr,
                                                    uint8_t chan_map,uint8_t filter_policy);
void btdrv_common_test_cmd_enable_ble_adv(uint8_t enable);
void btdrv_common_test_cmd_set_ble_adv_data(uint8_t adv_data_len,uint8_t *adv_data);
void btdrv_common_test_cmd_set_link_policy(uint16_t link_policy);
void btdrv_common_test_cmd_enter_sniff_mode(uint16_t conhandle,uint16_t max_interval,uint16_t min_interval,
                                                    uint16_t attempt,uint16_t timeout);
void btdrv_common_test_cmd_exit_sniff_mode(uint16_t conhandle);
void btdrv_common_test_cmd_autoaccept_connect(void);
void btdrv_common_test_cmd_bt_create_connect(uint8_t *addr,uint16_t pkt_type,uint8_t pscan_rep_mode,
                                                    uint16_t clk_off,uint8_t switch_en);
void btdrv_common_test_cmd_le_create_connect(uint16_t scan_interval,uint16_t scan_win,uint8_t filter_policy,
                                                    uint8_t peer_addr_type,uint8_t *peer_addr,uint8_t own_addr_type,
                                                    uint16_t conn_interval_min,uint16_t conn_interval_max,
                                                    uint16_t conn_latency,uint16_t connect_timeout,
                                                    uint16_t ce_len_min,uint16_t ce_len_max);
void btdrv_common_test_cmd_transmit_on(uint8_t bt_test_freq,uint8_t bt_test_pattern,uint8_t prb9_en,
                                                   uint8_t bt_test_txedr,uint8_t bt_test_rate,uint8_t bt_test_en);
void btdrv_common_test_cmd_receive_on(uint8_t bt_test_freq,uint8_t bt_test_rate,uint8_t bt_test_en);
void btdrv_common_test_cmd_set_ble_scan_param(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window,
                                            uint8_t own_addr_type, uint8_t scan_filter_policy);
void btdrv_common_test_cmd_ble_scan_en(void);


#ifdef __cplusplus
}
#endif
#endif
