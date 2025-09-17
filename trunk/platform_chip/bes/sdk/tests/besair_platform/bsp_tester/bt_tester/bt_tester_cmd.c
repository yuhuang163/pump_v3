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
#include "bt_tester_cmd.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "bt_drv.h"
#include "hal_sysfreq.h"
#include "common.h"

#define BTDIGITAL_REG_T(a) (*(volatile uint32_t*)(a))
extern uint8_t nosig_rx_test_flag;

static uint8_t basic_nosig_cmd[32] = {
    0x01, 0x87, 0xFC, 0x1C,
    0x00, //[4] Test scenario
    0xe8, 0x03, //[5]test_period
    0x00, //[7]hopping mode
    0x00, //[8]whitening mode
    0x00, //[9]transmit frequency
    0x00, //[10]Reception frequency
    0x00, //[11]power level
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, //[12]BD address
    0x01, //[18]Lt address
    0x00, //[19]EDR enable
    0x0F, //[20]packet type
    0x03, //[21]payload_pattern
    0x53, 0x01, //[22]payload_length
    0x00, 0x00, 0x00, 0x00, //[24]start_latency_add
    0xFF, 0xFF, 0xFF, 0xFF //[28]packet_num
};

const uint8_t bt_local_feature[] =
{
#if defined(__3M_PACK__)
    0xBF, 0xeE, 0xCD,0xFe,0xdb,0xFd,0x7b,0x87
#else
    0xBF, 0xeE, 0xCD,0xFa,0xdb,0xbd,0x7b,0x87

    //0xBF,0xFE,0xCD,0xFa,0xDB,0xFd,0x73,0x87   // disable simple pairing
#endif
};

uint8_t bt_tester_sleep_param[] =
{
    0,    // sleep_en;
    1,    // exwakeup_en;
    0xd0,0x7,    //  lpo_calib_interval;   lpo calibration interval
    0x32,0,0,0,    // lpo_calib_time;  lpo count lpc times
};

static uint16_t get_paylaod_size(uint8_t edr, uint8_t type)
{
    uint16_t paylaod_size = 0;
    switch (type) {
        case 0x04:
            if (edr)
                paylaod_size = 54;
            else
                paylaod_size = 27;
            break;
        case 0x08:
            if (edr)
                paylaod_size = 83;
            else
                paylaod_size = 10;
            break;
        case 0x0A:
            if (edr)
                paylaod_size = 367;
            else
                paylaod_size = 121;
            break;
        case 0x0B:
            if (edr)
                paylaod_size = 552;
            else
                paylaod_size = 183;
            break;
        case 0x0E:
            if (edr)
                paylaod_size = 679;
            else
                paylaod_size = 224;
            break;
        case 0x0F:
            if (edr)
                paylaod_size = 1021;
            else
                paylaod_size = 339;
            break;
        default:
            break;
    }
    return paylaod_size;
}


void bt_tester_cmd_hci_reset(void)
{
    uint8_t hci_reset_cmd[] = { 0x01, 0x03, 0x0C, 0x00 };

    btdrv_SendData(hci_reset_cmd, sizeof(hci_reset_cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_wr_bt_addr(uint8_t* addr)
{
    uint8_t cmd[11];

    cmd[0] = 0x01;
    cmd[1] = 0x72;
    cmd[2] = 0xFC;
    cmd[3] = 0x07;
    cmd[4] = 0x00;
    memcpy(&cmd[5], addr, 6);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_rd_bt_addr(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x09;
    cmd[2] = 0x10;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_wr_ble_pubaddr(uint8_t* addr)
{
    uint8_t cmd[11];

    cmd[0] = 0x01;
    cmd[1] = 0x72;
    cmd[2] = 0xFC;
    cmd[3] = 0x07;
    cmd[4] = 0x01;
    memcpy(&cmd[5], addr, 6);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_rd_ble_pubaddr(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x09;
    cmd[2] = 0x10;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_wr_ble_randaddr(uint8_t *addr)
{
    uint8_t cmd[10];

    cmd[0] = 0x01;
    cmd[1] = 0x05;
    cmd[2] = 0x20;
    cmd[3] = 0x06;
    memcpy(&cmd[4],addr,6);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_wr_bt_local_feat(void)
{
    uint8_t cmd[12];

    cmd[0] = 0x01;
    cmd[1] = 0x81;
    cmd[2] = 0xfc;
    cmd[3] = 0x08;
    memcpy(&cmd[4],bt_local_feature,8);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_rd_bt_local_feat(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x03;
    cmd[2] = 0x10;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}


void bt_tester_cmd_wr_pscan_activity(uint16_t pscan_interval, uint16_t pscan_window)
{
    uint8_t cmd[8];

    cmd[0] = 0x01;
    cmd[1] = 0x1c;
    cmd[2] = 0x0c;
    cmd[3] = 0x04;
    memcpy(&cmd[4],&pscan_interval,2);
    memcpy(&cmd[6],&pscan_window,2);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_rd_pscan_activity(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x1b;
    cmd[2] = 0x0c;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}


void bt_tester_cmd_wr_iscan_activity(uint16_t iscan_interval,uint16_t iscan_window)
{
    uint8_t cmd[8];

    cmd[0] = 0x01;
    cmd[1] = 0x1e;
    cmd[2] = 0x0c;
    cmd[3] = 0x04;
    memcpy(&cmd[4],&iscan_interval,2);
    memcpy(&cmd[6],&iscan_window,2);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_rd_iscan_activity(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x1d;
    cmd[2] = 0x0c;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_scan_control(enum BT_TESTER_SCAN_TYPE_T type)
{
    uint8_t cmd[5];

    cmd[0] = 0x01;
    cmd[1] = 0x1a;
    cmd[2] = 0x0c;
    cmd[3] = 0x01;
    switch(type)
    {
        case BT_TESTER_BOTH_SCAN_DISABLE:
            cmd[4] = 0x00;
            break;
        case BT_TESTER_INQUIRY_SCAN_ENABLE:
            cmd[4] = 0x01;
            break;
        case BT_TESTER_PAGE_SCAN_ENABLE:
            cmd[4] = 0x02;
            break;
        case BT_TESTER_BOTH_SCAN_ENABLE:
            cmd[4] = 0x03;
            break;
    }

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_set_bt_sleep(uint8_t enable)
{
    uint8_t cmd[12];

    bt_tester_sleep_param[0] = enable;
    cmd[0] = 0x01;
    cmd[1] = 0x77;
    cmd[2] = 0xfc;
    cmd[3] = 0x08;
    memcpy(&cmd[4],bt_tester_sleep_param,8);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_set_bt_power(struct btdrv_customer_rf_config_t* config)
{
    bt_drv_rf_set_customer_config(config);
    bt_drv_tx_pwr_init();
}

void bt_tester_cmd_set_ble_power(struct btdrv_customer_rf_config_t* config)
{
    bt_drv_rf_set_customer_config(config);
    bt_drv_tx_pwr_init();
}

void bt_tester_cmd_rd_bt_power(void)
{
    rx_agc_t rx_val;
    bt_drv_reg_op_read_rssi_in_dbm(0x80, &rx_val);
    CMD_TRACE("RSSI[%d] Rx Gain[%d]\n", rx_val.rssi, rx_val.rxgain);
}

void bt_tester_cmd_rd_ble_power(void)
{
    rx_agc_t rx_val;
    bt_drv_reg_op_read_ble_rssi_in_dbm(0x00, &rx_val);
    CMD_TRACE("RSSI[%d] Rx Gain[%d]\n", rx_val.rssi, rx_val.rxgain);
}

void bt_tester_cmd_le_tx_test_v1(uint8_t channel, uint8_t test_data_len, uint8_t pkt_payload)
{
    uint8_t cmd[7];

    cmd[0] = 0x01;
    cmd[1] = 0x1E;
    cmd[2] = 0x20;
    cmd[3] = 0x03;
    cmd[4] = channel;
    cmd[5] = test_data_len;
    cmd[6] = pkt_payload;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_le_tx_test_v2(uint8_t channel, uint8_t test_data_len,
    uint8_t pkt_payload, uint8_t phy)
{
    uint8_t cmd[8];

    cmd[0] = 0x01;
    cmd[1] = 0x34;
    cmd[2] = 0x20;
    cmd[3] = 0x04;
    cmd[4] = channel;
    cmd[5] = test_data_len;
    cmd[6] = pkt_payload;
    cmd[7] = phy;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_le_tx_test_v3(bt_tester_cmd_le_tx_test_v3_t param)
{
    uint8_t *le_tx_v3_cmd = NULL;
    le_tx_v3_cmd = (uint8_t *) malloc(BT_TESTER_CMD_BLE_TX_V3_CMD_MAX_LEN);
    uint8_t cmd_len = 0;
    
    le_tx_v3_cmd[0] = 0x01;
    le_tx_v3_cmd[1] = 0x50;
    le_tx_v3_cmd[2] = 0x20;
    le_tx_v3_cmd[3] = 7 + param.switching_pattern_len;
    le_tx_v3_cmd[4] = param.channel;
    le_tx_v3_cmd[5] = param.test_data_len;
    le_tx_v3_cmd[6] = param.pkt_payload;
    le_tx_v3_cmd[7] = param.phy;
    le_tx_v3_cmd[8] = param.cte_len;
    le_tx_v3_cmd[9] = param.cte_type;
    le_tx_v3_cmd[10] = param.switching_pattern_len;
    memcpy(&le_tx_v3_cmd[11], &param.antenna_id[0], param.switching_pattern_len);
    cmd_len = 11 + param.switching_pattern_len;

    btdrv_SendData(le_tx_v3_cmd, cmd_len);
    btdrv_delay(20);

    free(le_tx_v3_cmd);
}

void bt_tester_cmd_le_tx_test_v4(bt_tester_cmd_le_tx_test_v4_t param)
{
    uint8_t *le_tx_v4_cmd = NULL;
    le_tx_v4_cmd = (uint8_t *) malloc(BT_TESTER_CMD_BLE_TX_V4_CMD_MAX_LEN);
    uint8_t cmd_len = 0;
    uint8_t tx_pwr_lvl_idx = 0;
    
    le_tx_v4_cmd[0] = 0x01;
    le_tx_v4_cmd[1] = 0x7b;
    le_tx_v4_cmd[2] = 0x20;
    le_tx_v4_cmd[3] = 8 + param.switching_pattern_len;
    le_tx_v4_cmd[4] = param.channel;
    le_tx_v4_cmd[5] = param.test_data_len;
    le_tx_v4_cmd[6] = param.pkt_payload;
    le_tx_v4_cmd[7] = param.phy;
    le_tx_v4_cmd[8] = param.cte_len;
    le_tx_v4_cmd[9] = param.cte_type;
    le_tx_v4_cmd[10] = param.switching_pattern_len;
    memcpy(&le_tx_v4_cmd[11], &param.antenna_id[0], param.switching_pattern_len);
    tx_pwr_lvl_idx = 11 + param.switching_pattern_len;
    le_tx_v4_cmd[tx_pwr_lvl_idx] = param.power_level;
    cmd_len = 12 + param.switching_pattern_len;

    btdrv_SendData(le_tx_v4_cmd, cmd_len);
    btdrv_delay(20);

    free(le_tx_v4_cmd);
}

void bt_tester_cmd_le_rx_test_v1(uint8_t channel)
{
    uint8_t cmd[5];

    cmd[0] = 0x01;
    cmd[1] = 0x1D;
    cmd[2] = 0x20;
    cmd[3] = 0x01;
    cmd[4] = channel;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_le_rx_test_v2(uint8_t channel, uint8_t phy, uint8_t modulation_index)
{
    uint8_t cmd[7];

    cmd[0] = 0x01;
    cmd[1] = 0x33;
    cmd[2] = 0x20;
    cmd[3] = 0x03;
    cmd[4] = channel;
    cmd[5] = phy;
    cmd[6] = modulation_index;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_le_rx_test_v3(bt_tester_cmd_le_rx_test_v3_t param)
{
    uint8_t *le_rx_v3_cmd = NULL;
    le_rx_v3_cmd = (uint8_t *) malloc(BT_TESTER_CMD_BLE_RX_V3_CMD_MAX_LEN);
    uint8_t cmd_len = 0;

    le_rx_v3_cmd[0] = 0x01;
    le_rx_v3_cmd[1] = 0x4f;
    le_rx_v3_cmd[2] = 0x20;
    le_rx_v3_cmd[3] = 7 + param.switching_pattern_len;
    le_rx_v3_cmd[4] = param.channel;
    le_rx_v3_cmd[5] = param.phy;
    le_rx_v3_cmd[6] = param.modulation_idx;
    le_rx_v3_cmd[7] = param.exp_cte_len;
    le_rx_v3_cmd[8] = param.exp_cte_type;
    le_rx_v3_cmd[9] = param.slot_durations;
    le_rx_v3_cmd[10] = param.switching_pattern_len;
    memcpy(&le_rx_v3_cmd[11], &param.antenna_id[0], param.switching_pattern_len);
    cmd_len = 11 + param.switching_pattern_len;

    btdrv_SendData(le_rx_v3_cmd, cmd_len);
    btdrv_delay(20);

    free(le_rx_v3_cmd);
}

void bt_tester_cmd_le_test_end(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x1F;
    cmd[2] = 0x20;
    cmd[3] = 0x00;
    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_nosig_bdr_tx_test(uint8_t channel, uint8_t package_type,
    uint8_t payload_type, uint8_t power_level, uint8_t *addr)
{
    uint16_t paylaod_size;

    nosig_rx_test_flag = 0;

    basic_nosig_cmd[4] = 0x00;
    basic_nosig_cmd[19] = 0x00;
    basic_nosig_cmd[9] = channel;
    basic_nosig_cmd[11] = power_level;
    memcpy(&basic_nosig_cmd[12],addr,6);
    basic_nosig_cmd[20] = package_type;
    basic_nosig_cmd[21] = payload_type;
    paylaod_size = get_paylaod_size(basic_nosig_cmd[19], basic_nosig_cmd[20]);
    basic_nosig_cmd[22] = paylaod_size % 0x100;
    basic_nosig_cmd[23] = paylaod_size / 0x100;

    btdrv_SendData(basic_nosig_cmd, sizeof(basic_nosig_cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_nosig_edr_tx_test(uint8_t channel, uint8_t package_type,
    uint8_t payload_type, uint8_t power_level, uint8_t *addr)
{
    uint16_t paylaod_size;

    nosig_rx_test_flag = 0;

    basic_nosig_cmd[4] = 0x00;
    basic_nosig_cmd[19] = 0x01;
    basic_nosig_cmd[9] = channel;
    basic_nosig_cmd[11] = power_level;
    memcpy(&basic_nosig_cmd[12],addr,6);
    basic_nosig_cmd[20] = package_type;
    basic_nosig_cmd[21] = payload_type;
    paylaod_size = get_paylaod_size(basic_nosig_cmd[19], basic_nosig_cmd[20]);
    basic_nosig_cmd[22] = paylaod_size % 0x100;
    basic_nosig_cmd[23] = paylaod_size / 0x100;

    btdrv_SendData(basic_nosig_cmd, sizeof(basic_nosig_cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_nosig_bdr_rx_test(uint8_t channel, uint8_t package_type,
    uint8_t payload_type, uint8_t power_level, uint8_t *addr)
{
    uint16_t paylaod_size;

    nosig_rx_test_flag = 1;

    basic_nosig_cmd[4] = 0x01;
    basic_nosig_cmd[19] = 0x00;
    basic_nosig_cmd[10] = channel;
    basic_nosig_cmd[11] = power_level;
    memcpy(&basic_nosig_cmd[12],addr,6);
    basic_nosig_cmd[20] = package_type;
    basic_nosig_cmd[21] = payload_type;
    paylaod_size = get_paylaod_size(basic_nosig_cmd[19], basic_nosig_cmd[20]);
    basic_nosig_cmd[22] = paylaod_size % 0x100;
    basic_nosig_cmd[23] = paylaod_size / 0x100;

    btdrv_SendData(basic_nosig_cmd, sizeof(basic_nosig_cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_nosig_edr_rx_test(uint8_t channel, uint8_t package_type,
    uint8_t payload_type, uint8_t power_level, uint8_t *addr)
{
    uint16_t paylaod_size;

    nosig_rx_test_flag = 1;

    basic_nosig_cmd[4] = 0x01;
    basic_nosig_cmd[19] = 0x01;
    basic_nosig_cmd[10] = channel;
    basic_nosig_cmd[11] = power_level;
    memcpy(&basic_nosig_cmd[12],addr,6);
    basic_nosig_cmd[20] = package_type;
    basic_nosig_cmd[21] = payload_type;
    paylaod_size = get_paylaod_size(basic_nosig_cmd[19], basic_nosig_cmd[20]);
    basic_nosig_cmd[22] = paylaod_size % 0x100;
    basic_nosig_cmd[23] = paylaod_size / 0x100;

    btdrv_SendData(basic_nosig_cmd, sizeof(basic_nosig_cmd));
    btdrv_delay(20);
}

void bt_tester_cmd_nosig_stop_test(void)
{
    if(nosig_rx_test_flag)
    {
        nosig_rx_test_flag = 2;
    }

    basic_nosig_cmd[4] = 0x02;
    basic_nosig_cmd[19] = 0x00;
    basic_nosig_cmd[9] = 0x00;
    basic_nosig_cmd[10] = 0x00;
    basic_nosig_cmd[11] = 0x00;
    basic_nosig_cmd[20] = 0x0F;
    basic_nosig_cmd[21] = 0x04;
    basic_nosig_cmd[22] = 0x53;
    basic_nosig_cmd[23] = 0x01;

    btdrv_SendData(basic_nosig_cmd, sizeof(basic_nosig_cmd));
    btdrv_delay(20);
}

static void bt_rssi_dump_timer_handler(void const *param);
osTimerDef(bt_rssi_dump_timer, bt_rssi_dump_timer_handler);
static osTimerId bt_rssi_dump_timer_id = NULL;

static void bt_rssi_dump_timer_handler(void const *param)
{
    //dump rssi
    bt_drv_rssi_dump_handler();
}

void bt_tester_cmd_enter_signal_mode(void)
{
    btdrv_enable_dut();
    bt_rssi_dump_timer_id = osTimerCreate(osTimer(bt_rssi_dump_timer),osTimerPeriodic,NULL);
    osTimerStart(bt_rssi_dump_timer_id, 1000);
    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_26M);

}

void bt_tester_cmd_carrier_start(uint8_t channel)
{
    CMD_TRACE("btdrv_vco_test_start");
    btdrv_vco_test_start(channel);
}

void bt_tester_cmd_carrier_stop(void)
{
    CMD_TRACE("btdrv_vco_test_stop");
    btdrv_vco_test_stop();
}


