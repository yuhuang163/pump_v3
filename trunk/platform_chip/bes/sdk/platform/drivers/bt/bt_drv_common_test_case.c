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
#ifndef PROGRAMMER
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

typedef struct
{
    uint16_t    pkt_counters;
    uint16_t    head_errors;
    uint16_t    payload_errors;
    int16_t    avg_estsw;
    int16_t    avg_esttpl;
    uint32_t    payload_bit_errors;
}btdrv_common_test_nosig_test_evt_ind_t;

uint8_t btdrv_common_test_nosig_rx_test_flag = 0;
void btdrv_common_test_nonsig_test_result_save(const unsigned char *data, unsigned int len)
{
    int sRet = 0;
    uint32_t per = 0;
    uint32_t little = 0;

    btdrv_common_test_nosig_test_evt_ind_t base_test_result_bt;
    btdrv_common_test_nosig_test_evt_ind_t *pBaseResultBt = NULL;

    if(btdrv_common_test_nosig_rx_test_flag == 2)
    {
        btdrv_common_test_nosig_rx_test_flag = 0;
        pBaseResultBt = (btdrv_common_test_nosig_test_evt_ind_t *)(data + 7);
        if (pBaseResultBt->pkt_counters != 0) {
            sRet = memcpy_s(&base_test_result_bt, sizeof(base_test_result_bt), pBaseResultBt, sizeof(base_test_result_bt));
            if(sRet != 0)
            {
                TRACE(0,"func-s line:%d sRet:%d %s ", __LINE__, sRet, __func__);
            }

            TRACE(0,"btdrv_common_test nosig rx result:");
            TRACE(0,"Pkt Total Received [%d]", base_test_result_bt.pkt_counters);
            TRACE(0,"Pkt Head Error [%d]", base_test_result_bt.head_errors);
            TRACE(0,"Pkt Payload Error [%d]", base_test_result_bt.payload_errors);
            per = (1000 - base_test_result_bt.pkt_counters + base_test_result_bt.payload_errors + base_test_result_bt.head_errors) / 10;
            little = (1000 - base_test_result_bt.pkt_counters + base_test_result_bt.payload_errors + base_test_result_bt.head_errors) % 10;
            TRACE(0,"Per [%d.%d%%]", per, little);
        }
        else
        {
            TRACE(0,"btdrv_common_test nosig rx result: no pkt received");
        }
    }
}

void btdrv_common_test_cmd_receive_evt_analyze(const unsigned char *data, unsigned int len)
{
    if(data[0] == 0x04 && data[4] == 0x09 && data[5] == 0x10)
    {
        TRACE(0,"btdrv_common_test CMD_RD_ADDR:");
        DUMP8("0x%02x ",&data[7],6);//04 0e 0a 05 09 10 00 12 34 56 78 90 ab
    }

    if(data[0] == 0x04 && data[4] == 0x03 && data[5] == 0x10)
    {
        TRACE(0,"btdrv_common_test CMD_RD_BT_LOCAL_FEAT:");
        DUMP8("0x%02x ",&data[7],8);//04 0e 0c 05 03 10 00 bf ee cd fa db bd 7b 87
    }

    if(data[0] == 0x04 && data[4] == 0x1b && data[5] == 0x0c)
    {
        uint16_t pscan_interval = (uint16_t)(data[8] << 8) | data[7];
        uint16_t pscan_window = (uint16_t)(data[10] << 8) | data[9];
        TRACE(0,"btdrv_common_test CMD_RD_PSCAN_ACTIVITY: interval=0x%02x,window=0x%02x",pscan_interval,pscan_window);
        //04 0e 08 05 1b 0c 00 40 00 12 00
    }

    if(data[0] == 0x04 && data[4] == 0x1d && data[5] == 0x0c)
    {
        uint16_t iscan_interval = (uint16_t)(data[8] << 8) | data[7];
        uint16_t iscan_window = (uint16_t)(data[10] << 8) | data[9];
        TRACE(0,"btdrv_common_test CMD_RD_ISCAN_ACTIVITY: interval=0x%02x,window=0x%02x",iscan_interval,iscan_window);
        //04 0e 08 05 1d 0c 00 40 00 12 00
    }

    if(data[0] == 0x04 && data[4] == 0x1f && data[5] == 0x20)
    {
        uint16_t per = 0;
        uint16_t num_pkts = (uint16_t)(data[8] << 8) | data[7];
        per = 1000 - data[8] * 0x100 + data[7];
        TRACE(0,"btdrv_common_test CMD_BLE_TEST_END: num_pkts=0x%x, per=%d",num_pkts,per);//04 0e 06 05 1f 20 00 00 00
    }

    if(data[0] == 0x04 && data[4] == 0x87 && data[5] == 0xfc)
    {
        btdrv_common_test_nonsig_test_result_save(data,len);
    }
}

static uint8_t btdrv_common_test_basic_nosig_cmd[32] = {
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

const uint8_t btdrv_common_test_bt_local_feature[] =
{
#if defined(__3M_PACK__)
    0xBF, 0xeE, 0xCD,0xFe,0xdb,0xFd,0x7b,0x87
#else
    0xBF, 0xeE, 0xCD,0xFa,0xdb,0xbd,0x7b,0x87

    //0xBF,0xFE,0xCD,0xFa,0xDB,0xFd,0x73,0x87   // disable simple pairing
#endif
};

uint8_t btdrv_common_test_sleep_param[] =
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

void btdrv_common_test_cmd_hci_reset(void)
{
    uint8_t hci_reset_cmd[] = { 0x01, 0x03, 0x0C, 0x00 };

    btdrv_SendData(hci_reset_cmd, sizeof(hci_reset_cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_wr_bt_addr(uint8_t* addr)
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

void btdrv_common_test_cmd_rd_bt_addr(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x09;
    cmd[2] = 0x10;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_wr_ble_pubaddr(uint8_t* addr)
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

void btdrv_common_test_cmd_rd_ble_pubaddr(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x09;
    cmd[2] = 0x10;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_wr_ble_randaddr(uint8_t *addr)
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

void btdrv_common_test_cmd_wr_bt_local_feat(void)
{
    uint8_t cmd[12];

    cmd[0] = 0x01;
    cmd[1] = 0x81;
    cmd[2] = 0xfc;
    cmd[3] = 0x08;
    memcpy(&cmd[4],btdrv_common_test_bt_local_feature,8);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_rd_bt_local_feat(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x03;
    cmd[2] = 0x10;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}


void btdrv_common_test_cmd_wr_pscan_activity(uint16_t pscan_interval, uint16_t pscan_window)
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

void btdrv_common_test_cmd_rd_pscan_activity(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x1b;
    cmd[2] = 0x0c;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}


void btdrv_common_test_cmd_wr_iscan_activity(uint16_t iscan_interval,uint16_t iscan_window)
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

void btdrv_common_test_cmd_rd_iscan_activity(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x1d;
    cmd[2] = 0x0c;
    cmd[3] = 0x00;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_scan_control(enum BTDRV_COMMON_TEST_SCAN_TYPE_T type)
{
    uint8_t cmd[5];

    cmd[0] = 0x01;
    cmd[1] = 0x1a;
    cmd[2] = 0x0c;
    cmd[3] = 0x01;
    switch(type)
    {
        case BTDRV_COMMON_TEST_BOTH_SCAN_DISABLE:
            cmd[4] = 0x00;
            break;
        case BTDRV_COMMON_TEST_INQUIRY_SCAN_ENABLE:
            cmd[4] = 0x01;
            break;
        case BTDRV_COMMON_TEST_PAGE_SCAN_ENABLE:
            cmd[4] = 0x02;
            break;
        case BTDRV_COMMON_TEST_BOTH_SCAN_ENABLE:
            cmd[4] = 0x03;
            break;
    }

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_wr_pagescan_type(uint8_t pscan_type)
{
    uint8_t cmd[5];

    cmd[0] = 0x01;
    cmd[1] = 0x47;
    cmd[2] = 0x0c;
    cmd[3] = 0x01;
    cmd[4] = pscan_type;//standard_scan=0,interlaced_scan=1

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_set_bt_sleep(uint8_t enable)
{
    uint8_t cmd[12];

    btdrv_common_test_sleep_param[0] = enable;
    cmd[0] = 0x01;
    cmd[1] = 0x77;
    cmd[2] = 0xfc;
    cmd[3] = 0x08;
    memcpy(&cmd[4],btdrv_common_test_sleep_param,8);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

struct btdrv_customer_rf_config_t btdrv_common_test_rf_customer_config;
extern int8_t  btdrv_txpwr_conv_tbl[];
void btdrv_common_test_rf_sdk_init(int8_t bt_tx_pwr_fix_dbm,int8_t ble_tx_pwr_fix_dbm)
{
    TRACE(0,"[%s]",__func__);
    btdrv_common_test_rf_customer_config.config_xtal_en = false;//get value from NV
    btdrv_common_test_rf_customer_config.xtal_cap_val = 0;//get value from NV
    //config TX power in dbm
    btdrv_common_test_rf_customer_config.config_tx_pwr_en = true;
    btdrv_common_test_rf_customer_config.bt_tx_page_pwr = bt_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.bt_tx_page_high_pwr = bt_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.bt_tx_max_pwr  = bt_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.bt_tx_idx4_pwr = bt_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.bt_tx_idx3_pwr = bt_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.bt_tx_idx2_pwr = bt_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.bt_tx_idx1_pwr = bt_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.bt_tx_idx0_pwr = bt_tx_pwr_fix_dbm;//in dbm

    btdrv_common_test_rf_customer_config.le_tx_max_pwr  = ble_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.le_tx_idx4_pwr = ble_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.le_tx_idx3_pwr = ble_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.le_tx_idx2_pwr = ble_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.le_tx_idx1_pwr = ble_tx_pwr_fix_dbm;//in dbm
    btdrv_common_test_rf_customer_config.le_tx_idx0_pwr = ble_tx_pwr_fix_dbm;//in dbm

    //init BLE convert table
    btdrv_txpwr_conv_tbl[0] = btdrv_common_test_rf_customer_config.le_tx_idx0_pwr;
    btdrv_txpwr_conv_tbl[1] = btdrv_common_test_rf_customer_config.le_tx_idx1_pwr;
    btdrv_txpwr_conv_tbl[2] = btdrv_common_test_rf_customer_config.le_tx_idx2_pwr;
    btdrv_txpwr_conv_tbl[3] = btdrv_common_test_rf_customer_config.le_tx_idx3_pwr;
    btdrv_txpwr_conv_tbl[4] = btdrv_common_test_rf_customer_config.le_tx_idx4_pwr;
    btdrv_txpwr_conv_tbl[5] = btdrv_common_test_rf_customer_config.le_tx_max_pwr;
    btdrv_txpwr_conv_tbl[6] = btdrv_common_test_rf_customer_config.le_tx_max_pwr;
    btdrv_txpwr_conv_tbl[7] = btdrv_common_test_rf_customer_config.le_tx_max_pwr;
}

void btdrv_common_test_cmd_bt_ble_tx_power_fix(int8_t bt_tx_pwr_fix_dbm,int8_t ble_tx_pwr_fix_dbm)
{
    btdrv_common_test_rf_sdk_init(bt_tx_pwr_fix_dbm,ble_tx_pwr_fix_dbm);
    bt_drv_rf_set_customer_config(&btdrv_common_test_rf_customer_config);
    bt_drv_tx_pwr_init();
}

void btdrv_common_test_cmd_rd_bt_power(uint16_t conhandle)
{
    rx_agc_t rx_val;
    bt_drv_reg_op_read_rssi_in_dbm(conhandle, &rx_val);
    TRACE(0,"[%s] RSSI[%d] Rx Gain[%d]\n", __func__, rx_val.rssi, rx_val.rxgain);
}

void btdrv_common_test_cmd_rd_ble_power(uint16_t conhandle)
{
    rx_agc_t rx_val;
    bt_drv_reg_op_read_ble_rssi_in_dbm(conhandle, &rx_val);
    TRACE(0,"[%s] RSSI[%d] Rx Gain[%d]\n", __func__, rx_val.rssi, rx_val.rxgain);
}

void btdrv_common_test_cmd_enable_bt_ble_rx_gain_fix(uint8_t enable,uint8_t bt_ble_mode,uint8_t gain_idx)
{
    btdrv_common_test_rx_gain_fix(bt_ble_mode,gain_idx,enable,0);
    btdrv_common_test_rx_gain_fix(bt_ble_mode,gain_idx,enable,1);
    btdrv_common_test_rx_gain_fix(bt_ble_mode,gain_idx,enable,2);
}

void btdrv_common_test_cmd_le_tx_test_v1(uint8_t channel, uint8_t test_data_len, uint8_t pkt_payload)
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

void btdrv_common_test_cmd_le_tx_test_v2(uint8_t channel, uint8_t test_data_len,
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

void btdrv_common_test_cmd_le_tx_test_v3(btdrv_common_test_cmd_le_tx_test_v3_t param)
{
#ifndef NOSTD
    uint8_t *le_tx_v3_cmd = NULL;
    le_tx_v3_cmd = (uint8_t *) malloc(BTDRV_COMMON_TEST_CMD_BLE_TX_V3_CMD_MAX_LEN);
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
#endif
}

void btdrv_common_test_cmd_le_tx_test_v4(btdrv_common_test_cmd_le_tx_test_v4_t param)
{
#ifndef NOSTD
    uint8_t *le_tx_v4_cmd = NULL;
    le_tx_v4_cmd = (uint8_t *) malloc(BTDRV_COMMON_TEST_CMD_BLE_TX_V4_CMD_MAX_LEN);
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
#endif
}

void btdrv_common_test_cmd_le_rx_test_v1(uint8_t channel)
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

void btdrv_common_test_cmd_le_rx_test_v2(uint8_t channel, uint8_t phy, uint8_t modulation_index)
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

void btdrv_common_test_cmd_le_rx_test_v3(btdrv_common_test_cmd_le_rx_test_v3_t param)
{
#ifndef NOSTD
    uint8_t *le_rx_v3_cmd = NULL;
    le_rx_v3_cmd = (uint8_t *) malloc(BTDRV_COMMON_TEST_CMD_BLE_RX_V3_CMD_MAX_LEN);
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
#endif
}

void btdrv_common_test_cmd_le_test_end(void)
{
    uint8_t cmd[4];

    cmd[0] = 0x01;
    cmd[1] = 0x1F;
    cmd[2] = 0x20;
    cmd[3] = 0x00;
    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_nosig_bdr_tx_test(uint8_t channel, uint8_t package_type,
    uint8_t payload_type, uint8_t power_level, uint8_t *addr)
{
    uint16_t paylaod_size;

    btdrv_common_test_nosig_rx_test_flag = 0;

    btdrv_common_test_basic_nosig_cmd[4] = 0x00;
    btdrv_common_test_basic_nosig_cmd[19] = 0x00;
    btdrv_common_test_basic_nosig_cmd[9] = channel;
    btdrv_common_test_basic_nosig_cmd[11] = power_level;
    memcpy(&btdrv_common_test_basic_nosig_cmd[12],addr,6);
    btdrv_common_test_basic_nosig_cmd[20] = package_type;
    btdrv_common_test_basic_nosig_cmd[21] = payload_type;
    paylaod_size = get_paylaod_size(btdrv_common_test_basic_nosig_cmd[19], btdrv_common_test_basic_nosig_cmd[20]);
    btdrv_common_test_basic_nosig_cmd[22] = paylaod_size % 0x100;
    btdrv_common_test_basic_nosig_cmd[23] = paylaod_size / 0x100;

    btdrv_SendData(btdrv_common_test_basic_nosig_cmd, sizeof(btdrv_common_test_basic_nosig_cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_nosig_edr_tx_test(uint8_t channel, uint8_t package_type,
    uint8_t payload_type, uint8_t power_level, uint8_t *addr)
{
    uint16_t paylaod_size;

    btdrv_common_test_nosig_rx_test_flag = 0;

    btdrv_common_test_basic_nosig_cmd[4] = 0x00;
    btdrv_common_test_basic_nosig_cmd[19] = 0x01;
    btdrv_common_test_basic_nosig_cmd[9] = channel;
    btdrv_common_test_basic_nosig_cmd[11] = power_level;
    memcpy(&btdrv_common_test_basic_nosig_cmd[12],addr,6);
    btdrv_common_test_basic_nosig_cmd[20] = package_type;
    btdrv_common_test_basic_nosig_cmd[21] = payload_type;
    paylaod_size = get_paylaod_size(btdrv_common_test_basic_nosig_cmd[19], btdrv_common_test_basic_nosig_cmd[20]);
    btdrv_common_test_basic_nosig_cmd[22] = paylaod_size % 0x100;
    btdrv_common_test_basic_nosig_cmd[23] = paylaod_size / 0x100;

    btdrv_SendData(btdrv_common_test_basic_nosig_cmd, sizeof(btdrv_common_test_basic_nosig_cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_nosig_bdr_rx_test(uint8_t channel, uint8_t package_type,
    uint8_t payload_type, uint8_t power_level, uint8_t *addr)
{
    uint16_t paylaod_size;

    btdrv_common_test_nosig_rx_test_flag = 1;

    btdrv_common_test_basic_nosig_cmd[4] = 0x01;
    btdrv_common_test_basic_nosig_cmd[19] = 0x00;
    btdrv_common_test_basic_nosig_cmd[10] = channel;
    btdrv_common_test_basic_nosig_cmd[11] = power_level;
    memcpy(&btdrv_common_test_basic_nosig_cmd[12],addr,6);
    btdrv_common_test_basic_nosig_cmd[20] = package_type;
    btdrv_common_test_basic_nosig_cmd[21] = payload_type;
    paylaod_size = get_paylaod_size(btdrv_common_test_basic_nosig_cmd[19], btdrv_common_test_basic_nosig_cmd[20]);
    btdrv_common_test_basic_nosig_cmd[22] = paylaod_size % 0x100;
    btdrv_common_test_basic_nosig_cmd[23] = paylaod_size / 0x100;

    btdrv_SendData(btdrv_common_test_basic_nosig_cmd, sizeof(btdrv_common_test_basic_nosig_cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_nosig_edr_rx_test(uint8_t channel, uint8_t package_type,
    uint8_t payload_type, uint8_t power_level, uint8_t *addr)
{
    uint16_t paylaod_size;

    btdrv_common_test_nosig_rx_test_flag = 1;

    btdrv_common_test_basic_nosig_cmd[4] = 0x01;
    btdrv_common_test_basic_nosig_cmd[19] = 0x01;
    btdrv_common_test_basic_nosig_cmd[10] = channel;
    btdrv_common_test_basic_nosig_cmd[11] = power_level;
    memcpy(&btdrv_common_test_basic_nosig_cmd[12],addr,6);
    btdrv_common_test_basic_nosig_cmd[20] = package_type;
    btdrv_common_test_basic_nosig_cmd[21] = payload_type;
    paylaod_size = get_paylaod_size(btdrv_common_test_basic_nosig_cmd[19], btdrv_common_test_basic_nosig_cmd[20]);
    btdrv_common_test_basic_nosig_cmd[22] = paylaod_size % 0x100;
    btdrv_common_test_basic_nosig_cmd[23] = paylaod_size / 0x100;

    btdrv_SendData(btdrv_common_test_basic_nosig_cmd, sizeof(btdrv_common_test_basic_nosig_cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_nosig_stop_test(void)
{
    if(btdrv_common_test_nosig_rx_test_flag)
    {
        btdrv_common_test_nosig_rx_test_flag = 2;
    }

    btdrv_common_test_basic_nosig_cmd[4] = 0x02;
    btdrv_common_test_basic_nosig_cmd[19] = 0x00;
    btdrv_common_test_basic_nosig_cmd[9] = 0x00;
    btdrv_common_test_basic_nosig_cmd[10] = 0x00;
    btdrv_common_test_basic_nosig_cmd[11] = 0x00;
    btdrv_common_test_basic_nosig_cmd[20] = 0x0F;
    btdrv_common_test_basic_nosig_cmd[21] = 0x04;
    btdrv_common_test_basic_nosig_cmd[22] = 0x53;
    btdrv_common_test_basic_nosig_cmd[23] = 0x01;

    btdrv_SendData(btdrv_common_test_basic_nosig_cmd, sizeof(btdrv_common_test_basic_nosig_cmd));
    btdrv_delay(20);
}

#ifdef RTOS
static void btdrv_common_test_rssi_dump_timer_handler(void const *param);
static const osTimerAttr_t TimerAttr_btdrv_common = {
    .name = "btdrv_common_test_rssi_dump_timer",
    .attr_bits = 0U,
    .cb_mem = NULL,
    .cb_size = 0U,
};
static osTimerId_t btdrv_common_test_rssi_dump_timer_id = NULL;

static void btdrv_common_test_rssi_dump_timer_handler(void const *param)
{
    //dump rssi
    bt_drv_rssi_dump_handler();
}
#endif

void btdrv_common_test_cmd_enter_signal_mode(void)
{
    btdrv_enable_dut();
#ifdef RTOS
    btdrv_common_test_rssi_dump_timer_id = osTimerNew((osTimerFunc_t)btdrv_common_test_rssi_dump_timer_handler,
                                                       osTimerPeriodic, NULL, &TimerAttr_btdrv_common);
    osTimerStart(btdrv_common_test_rssi_dump_timer_id, 1000);
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_26M);

}

void btdrv_common_test_cmd_carrier_start(uint8_t channel)
{
    TRACE(0,"[%s] btdrv_vco_test_start",__func__);
    btdrv_vco_test_start(channel);
}

void btdrv_common_test_cmd_carrier_stop(void)
{
    TRACE(0,"[%s] btdrv_vco_test_stop",__func__);
    btdrv_vco_test_stop();
}

void btdrv_common_test_cmd_ble_set_adv_param(uint16_t adv_interval_min,uint16_t adv_interval_max,uint8_t adv_type,
                                                    uint8_t own_addr_type,uint8_t peer_addr_type,uint8_t *peer_addr,
                                                    uint8_t chan_map,uint8_t filter_policy)
{
    uint8_t cmd[19];

    cmd[0] = 0x01;
    cmd[1] = 0x06;
    cmd[2] = 0x20;
    cmd[3] = 0x0f;
    cmd[4] = (uint8_t)adv_interval_min & 0xff;
    cmd[5] = (uint8_t)((adv_interval_min & 0xff00)>>8);
    cmd[6] = (uint8_t)adv_interval_max & 0xff;
    cmd[7] = (uint8_t)((adv_interval_max & 0xff00)>>8);
    cmd[8] = adv_type;
    cmd[9] = own_addr_type;
    cmd[10] = peer_addr_type;
    memcpy(&cmd[11],&peer_addr[0],6);
    cmd[17] = chan_map;
    cmd[18] = filter_policy;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_enable_ble_adv(uint8_t enable)
{
    uint8_t cmd[5];

    cmd[0] = 0x01;
    cmd[1] = 0x0a;
    cmd[2] = 0x20;
    cmd[3] = 0x01;
    cmd[4] = enable;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_set_ble_adv_data(uint8_t adv_data_len,uint8_t *adv_data)
{
    uint8_t set_adv_data_cmd[] = { 0x01, 0x08, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    set_adv_data_cmd[4] = adv_data_len;
    memcpy(&set_adv_data_cmd[5],&adv_data[0],adv_data_len);

    btdrv_SendData(set_adv_data_cmd, sizeof(set_adv_data_cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_set_link_policy(uint16_t link_policy)
{
    uint8_t cmd[6];

    cmd[0] = 0x01;
    cmd[1] = 0x0f;
    cmd[2] = 0x08;
    cmd[3] = 0x02;
    cmd[4] = (uint8_t)link_policy & 0xff;
    cmd[5] = (uint8_t)((link_policy & 0xff00)>>8);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_enter_sniff_mode(uint16_t conhandle,uint16_t max_interval,uint16_t min_interval,
                                                    uint16_t attempt,uint16_t timeout)
{
    uint8_t cmd[14];

    cmd[0] = 0x01;
    cmd[1] = 0x03;
    cmd[2] = 0x08;
    cmd[3] = 0x0a;
    cmd[4] = (uint8_t)conhandle & 0xff;
    cmd[5] = (uint8_t)((conhandle & 0xff00)>>8);
    cmd[6] = (uint8_t)max_interval & 0xff;
    cmd[7] = (uint8_t)((max_interval & 0xff00)>>8);
    cmd[8] = (uint8_t)min_interval & 0xff;
    cmd[9] = (uint8_t)((min_interval & 0xff00)>>8);
    cmd[10] = (uint8_t)attempt & 0xff;
    cmd[11] = (uint8_t)((attempt & 0xff00)>>8);
    cmd[12] = (uint8_t)timeout & 0xff;
    cmd[13] = (uint8_t)((timeout & 0xff00)>>8);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_exit_sniff_mode(uint16_t conhandle)
{
    uint8_t cmd[6];

    cmd[0] = 0x01;
    cmd[1] = 0x04;
    cmd[2] = 0x08;
    cmd[3] = 0x02;
    cmd[4] = (uint8_t)conhandle & 0xff;
    cmd[5] = (uint8_t)((conhandle & 0xff00)>>8);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_autoaccept_connect(void)
{
    uint8_t auto_accept_cmd[] = { 0x01,0x05, 0x0c, 0x03, 0x02, 0x00, 0x02 };

    btdrv_SendData(auto_accept_cmd, sizeof(auto_accept_cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_bt_create_connect(uint8_t *addr,uint16_t pkt_type,uint8_t pscan_rep_mode,
                                                    uint16_t clk_off,uint8_t switch_en)
{
    uint8_t cmd[17];

    cmd[0] = 0x01;
    cmd[1] = 0x05;
    cmd[2] = 0x04;
    cmd[3] = 0x0d;
    memcpy(&cmd[4],&addr[0],6);
    cmd[10] = (uint8_t)pkt_type & 0xff;
    cmd[11] = (uint8_t)((pkt_type & 0xff00)>>8);
    cmd[12] = pscan_rep_mode;
    cmd[13] = 0x00;
    cmd[14] = (uint8_t)clk_off & 0xff;
    cmd[15] = (uint8_t)((clk_off & 0xff00)>>8);
    cmd[16] = switch_en;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_le_create_connect(uint16_t scan_interval,uint16_t scan_win,uint8_t filter_policy,
                                                    uint8_t peer_addr_type,uint8_t *peer_addr,uint8_t own_addr_type,
                                                    uint16_t conn_interval_min,uint16_t conn_interval_max,
                                                    uint16_t conn_latency,uint16_t connect_timeout,
                                                    uint16_t ce_len_min,uint16_t ce_len_max)
{
    uint8_t cmd[29];

    cmd[0] = 0x01;
    cmd[1] = 0x0d;
    cmd[2] = 0x20;
    cmd[3] = 0x19;
    cmd[4] = (uint8_t)scan_interval & 0xff;
    cmd[5] = (uint8_t)((scan_interval & 0xff00)>>8);
    cmd[6] = (uint8_t)scan_win & 0xff;
    cmd[7] = (uint8_t)((scan_win & 0xff00)>>8);
    cmd[8] = filter_policy;
    cmd[9] = peer_addr_type;
    memcpy(&cmd[10],&peer_addr[0],6);
    cmd[16] = own_addr_type;
    cmd[17] = (uint8_t)conn_interval_min & 0xff;
    cmd[18] = (uint8_t)((conn_interval_min & 0xff00)>>8);
    cmd[19] = (uint8_t)conn_interval_max & 0xff;
    cmd[20] = (uint8_t)((conn_interval_max & 0xff00)>>8);
    cmd[21] = (uint8_t)conn_latency & 0xff;
    cmd[22] = (uint8_t)((conn_latency & 0xff00)>>8);
    cmd[23] = (uint8_t)connect_timeout & 0xff;
    cmd[24] = (uint8_t)((connect_timeout & 0xff00)>>8);
    cmd[25] = (uint8_t)ce_len_min & 0xff;
    cmd[26] = (uint8_t)((ce_len_min & 0xff00)>>8);
    cmd[27] = (uint8_t)ce_len_max & 0xff;
    cmd[28] = (uint8_t)((ce_len_max & 0xff00)>>8);

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

#define TRANSMIT_RECEIVE_ON_TEST_ADDR        0xd0220c00//5.2 chip addr

void btdrv_common_test_cmd_transmit_on(uint8_t bt_test_freq,uint8_t bt_test_pattern,uint8_t prb9_en,
                                                   uint8_t bt_test_txedr,uint8_t bt_test_rate,uint8_t bt_test_en)
{
    btdrv_sleep_config(0);
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0x7f, 0, bt_test_freq);//bt_test_freq
    BTDIGITAL_REG(TRANSMIT_RECEIVE_ON_TEST_ADDR) &= 0xffffff7f;//bt_txrx_mode rx:1 tx:0
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0xff, 8, bt_test_pattern);//bt_test_pattern
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0x1, 16, prb9_en);
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0x1, 18, bt_test_txedr);//bt_test_txedr edr:1 bdr:0
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0x3, 19, bt_test_rate);//bt_test_rate 0:1m 1:2m 2:3m
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0x1, 17, bt_test_en);//bt_test_en
    TRACE(0,"TX ON txrx_test_addr=0x%x value=0x%x", TRANSMIT_RECEIVE_ON_TEST_ADDR, BTDIGITAL_REG(TRANSMIT_RECEIVE_ON_TEST_ADDR));
}

void btdrv_common_test_cmd_receive_on(uint8_t bt_test_freq,uint8_t bt_test_rate,uint8_t bt_test_en)
{
    btdrv_sleep_config(0);
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0x7f, 0, bt_test_freq);//bt_test_freq
    BTDIGITAL_REG(TRANSMIT_RECEIVE_ON_TEST_ADDR) |= (1<<7);//bt_txrx_mode rx:1 tx:0
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0x3, 19, bt_test_rate);//bt_test_rate 0:1m 1:2m 2:3m
    BTDIGITAL_REG_SET_FIELD(TRANSMIT_RECEIVE_ON_TEST_ADDR, 0x1, 17, bt_test_en);//bt_test_en
    TRACE(0,"RX ON txrx_test_addr=0x%x value=0x%x", TRANSMIT_RECEIVE_ON_TEST_ADDR, BTDIGITAL_REG(TRANSMIT_RECEIVE_ON_TEST_ADDR));
}

void btdrv_common_test_cmd_set_ble_scan_param(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window,
                                            uint8_t own_addr_type, uint8_t scan_filter_policy)
{
    uint8_t cmd[11];

    cmd[0] = 0x01;
    cmd[1] = 0x0b;
    cmd[2] = 0x20;
    cmd[3] = 0x07;
    cmd[4] = scan_type;
    cmd[5] = (uint8_t)scan_interval & 0xff;
    cmd[6] = (uint8_t)((scan_interval & 0xff00)>>8);
    cmd[7] = (uint8_t)scan_window & 0xff;
    cmd[8] = (uint8_t)((scan_window & 0xff00)>>8);
    cmd[9] = own_addr_type;
    cmd[10] = scan_filter_policy;

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

void btdrv_common_test_cmd_ble_scan_en(void)
{
    uint8_t cmd[6];

    cmd[0] = 0x01;
    cmd[1] = 0x0c;
    cmd[2] = 0x20;
    cmd[3] = 0x02;
    cmd[4] = 0x01;//enable
    cmd[5] = 0x01;//filter duplicates

    btdrv_SendData(cmd, sizeof(cmd));
    btdrv_delay(20);
}

#endif