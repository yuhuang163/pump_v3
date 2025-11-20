/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "string.h"
#ifndef PROGRAMMER
#include "math.h"
#endif
#include "bt_drv.h"
#include "bt_drv_internal.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "hal_intersys.h"
#include "hal_trace.h"
#include "hal_psc.h"
#include "hal_sysfreq.h"
#include "hal_chipid.h"
#include "pmu.h"
#include "nvrecord_dev.h"
#ifdef BTC_CPUDUMP_BASE
#include "bt_drv_cpudump.h"
#endif

#ifdef __BT_RAMRUN_NEW__
#include "bt_drv_ramrun_symbol_1503.h"
#include "../../bt_controller/bt_controller.h"
#endif

#ifdef DPD_ONCHIP_CAL
#include "bt_drv_dpd_mem.h"
#endif
// CHIP related
#include "bt_drv_1503_internal.h"
#include "bt_drv_1503_config.h"
#include "bt_1503_reg_map.h"
#include CHIP_SPECIFIC_HDR(bt_drv_modem_reg_map)
bool btdrv_dut_mode_enable = false;
static volatile uint32_t btdrv_tx_flag = 1;

void btdrv_tx(const unsigned char *data, unsigned int len)
{
    BT_DRV_TRACE(0,"tx");
    btdrv_tx_flag = 1;
}

#ifdef NORMAL_TEST_MODE_SWITCH
typedef int (*btdrv_testmode_nonsig_rx_process_func)(const unsigned char *data, unsigned int len);
btdrv_testmode_nonsig_rx_process_func btdrv_nonsig_test_result_callback = NULL;
void btdrv_testmode_register_nonsig_rx_process(btdrv_testmode_nonsig_rx_process_func cb)
{
    btdrv_nonsig_test_result_callback = cb;
}

static void btdrv_testmode_nonsig_rx_handler(const unsigned char *data, unsigned int len)
{
    if(btdrv_nonsig_test_result_callback !=NULL)
    {
        btdrv_nonsig_test_result_callback(data, len);
    }
}
#endif

#ifdef REDUCE_EDGE_CHL_TXPWR
// Reduce the edge channel tx power(chl0 & chl78), only single hop, optimize DEVM
static bool btdrv_reduce_edge_chl_txpwr_flag = false;
#define REDUCE_EDGE_CHL_TXPWR_1P5_DBM       3
#define BT_TX_TEST_CHL_0                    0x00
#define BT_TX_TEST_CHL_39                   0x27
#define BT_TX_TEST_CHL_78                   0x4e

void btdrv_reduce_edge_chl_txpwr_set(bool btdrv_reduce_edge_chl_txpwr_flag)
{
    uint16_t gsg_nom_q;
    uint16_t gsg_nom_i;
    uint16_t dsg_nom;
    uint16_t gsg_edr_nom;

    if (btdrv_reduce_edge_chl_txpwr_flag == false) {
        gsg_nom_i = besmdm_gfsk_dsg_nom_i_getf();
        gsg_nom_q = besmdm_gfsk_dsg_nom_q_getf();
        dsg_nom = besmdm_dpsk_dsg_nom_getf();
        gsg_edr_nom = besmdm_gsg_edr_nom_getf();
        gsg_nom_q -= REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        gsg_nom_i -= REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        dsg_nom -= REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        gsg_edr_nom -= REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        besmdm_dpsk_dsg_nom_setf(dsg_nom);
        besmdm_gfsk_dsg_nom_q_setf(gsg_nom_q);
        besmdm_gfsk_dsg_nom_i_setf(gsg_nom_i);
        besmdm_gsg_edr_nom_setf(gsg_edr_nom);
    } else if (btdrv_reduce_edge_chl_txpwr_flag == true) {
        gsg_nom_i = besmdm_gfsk_dsg_nom_i_getf();
        gsg_nom_q = besmdm_gfsk_dsg_nom_q_getf();
        dsg_nom = besmdm_dpsk_dsg_nom_getf();
        gsg_edr_nom = besmdm_gsg_edr_nom_getf();
        gsg_nom_q += REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        gsg_nom_i += REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        dsg_nom += REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        gsg_edr_nom += REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        besmdm_dpsk_dsg_nom_setf(dsg_nom);
        besmdm_gfsk_dsg_nom_q_setf(gsg_nom_q);
        besmdm_gfsk_dsg_nom_i_setf(gsg_nom_i);
        besmdm_gsg_edr_nom_setf(gsg_edr_nom);
    }
}

void btdrv_reduce_edge_chl_txpwr_signal_mode(const unsigned char *data)
{
    if (data[0] == 0x04 && data[1] == 0xff && data[3] == 0x18 && data[6] == 0x00 && (data[4] == BT_TX_TEST_CHL_0 || data[4] == BT_TX_TEST_CHL_78) && btdrv_reduce_edge_chl_txpwr_flag == false) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = true;
    } else if ((data[0] == 0x04 && data[1] == 0xff && data[3] == 0x18 && btdrv_reduce_edge_chl_txpwr_flag == true) && (data[6] == 0x01 || (data[4] != BT_TX_TEST_CHL_0 && data[4] != BT_TX_TEST_CHL_78))) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = false;
    }
}

void btdrv_reduce_edge_chl_txpwr_nosignal_bt_mode(const unsigned char *data)
{
    if (data[0] == 0x01 && data[1] == 0x87 && data[2] == 0xfc && data[3] == 0x1c && data[4] == 0x00 && (data[10] == BT_TX_TEST_CHL_0 || data[10] == BT_TX_TEST_CHL_78) && btdrv_reduce_edge_chl_txpwr_flag == false) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = true;
    } else if (data[0] == 0x01 && data[1] == 0x87 && data[2] == 0xfc && data[3] == 0x1c && data[4] == 0x00 && data[10] != BT_TX_TEST_CHL_0 && data[10] != BT_TX_TEST_CHL_78 && btdrv_reduce_edge_chl_txpwr_flag == true) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = false;
    }
}

void btdrv_reduce_edge_chl_txpwr_nosignal_ble_mode(const unsigned char *data)
{
    if (data[0] == 0x01 && data[1] == 0x1e && data[3] == 0x03 && data[6] == 0x02 && (data[4] == BT_TX_TEST_CHL_0 || data[4] == BT_TX_TEST_CHL_39) && btdrv_reduce_edge_chl_txpwr_flag == false) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = true;
    } else if (data[0] == 0x01 && data[1] == 0x1e && data[3] == 0x03 && data[6] == 0x02 && btdrv_reduce_edge_chl_txpwr_flag == true && data[4] != BT_TX_TEST_CHL_0 && data[4] != BT_TX_TEST_CHL_39) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = false;
    }
}
#endif

#ifdef __HW_AGC__
void btdrv_hwagc_lock_mode(const unsigned char *data)
{
    if (data[0] == 0x01 && data[1] == 0x87 && data[2] == 0xfc && data[3] == 0x1c && data[4] == 01)
    {
        BTRF_REG_SET_FIELD(0x37D, 0xF, 7, 0x0);
    }
    else if (data[0] == 0x01 && data[1] == 0x87 && data[2] == 0xfc && data[3] == 0x1c && data[4] == 02)
    {
        BTRF_REG_SET_FIELD(0x37D, 0xF, 7, 0xF);
    }
}
#endif

void btdrv_dut_accessible_mode_manager(const unsigned char *data);

static unsigned int btdrv_rx(const unsigned char *data, unsigned int len)
{
    hal_intersys_stop_recv(HAL_INTERSYS_ID_0);
    if (len > 5 && data[0] == 0x04 && data[1] == 0xff &&
        data[3] == 0x01)
    {
        BT_DRV_TRACE(2, "%s", &data[5]);
    }
    else
    {
        BT_DRV_TRACE(2, "%s len:%d", __func__, len);
        BT_DRV_DUMP("%02x ", data, len > 15 ? 15 : len);
    }

    bt_tester_cmd_receive_evt_analyze(data, len);

#ifdef NORMAL_TEST_MODE_SWITCH
    btdrv_testmode_nonsig_rx_handler(data, len);
#endif
    btdrv_dut_accessible_mode_manager(data);
    hal_intersys_start_recv(HAL_INTERSYS_ID_0);

#ifdef REDUCE_EDGE_CHL_TXPWR
    btdrv_reduce_edge_chl_txpwr_signal_mode(data);
    btdrv_reduce_edge_chl_txpwr_nosignal_ble_mode(data);
#endif

    return len;
}

////open intersys interface for hci data transfer
static bool hci_has_opened = false;

void btdrv_SendData(const uint8_t *buff, uint8_t len)
{
    if (hci_has_opened)
    {
        btdrv_tx_flag = 0;

        BT_DRV_TRACE(1, "%s", __func__);
        BT_DRV_DUMP("%02x ", buff, len);
        hal_intersys_send(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, buff, len);

        while ((btdrv_dut_mode_enable == 0) && btdrv_tx_flag == 0);
    }
    else
    {
        // only for bridge mode
        btdrv_bridge_send_data(buff, len);
        btdrv_delay(50);
    }
}

void btdrv_hciopen(void)
{
    int ret = 0;

    if (hci_has_opened)
    {
        return;
    }

    hci_has_opened = true;

    ret = hal_intersys_open(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, btdrv_rx, btdrv_tx, false);

    if (ret)
    {
        BT_DRV_TRACE(0, "Failed to open intersys");
        return;
    }

    hal_intersys_start_recv(HAL_INTERSYS_ID_0);
}

////open intersys interface for hci data transfer
void btdrv_hcioff(void)
{
    if (!hci_has_opened)
    {
        return;
    }
    hci_has_opened = false;

    hal_intersys_close(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI);
}

/*  btdrv power on or off the bt controller*/
void btdrv_poweron(uint8_t en)
{
#if !defined(CHIP_SUBSYS_SENS)
    pmu_bt_2v5pa(en);
#endif
    if (en) {
        hal_psc_bt_enable();
        hal_cmu_bt_clock_enable();
        hal_cmu_bt_reset_clear();
        hal_cmu_bt_module_init();
        btdrv_delay(10);
    } else {
        hal_cmu_bt_reset_set();
        hal_cmu_bt_clock_disable();
        hal_psc_bt_disable();
    }
}

void bt_drv_extra_config_after_init(void)
{
}

const uint16_t dc_iq_rf_set_common[][2] =
{
    {0x0024, 0x0070}, // txgain table 3	bit7:5 3’b011;	bit4 1;
    {0x0051, 0x8000}, // lna_hg_en_dr bit15 1
    {0x0052, 0xB096}, // lna_hg/dr/lna_hg_en bit10:5 000100

    {0x040b, 0x0001}, // pa_short_en dr		bit1 0; bit0 1;
    {0x0408, 0x1005}, // i2v_trx_mode dr	bit12 1; bit3 0; bit2 1; BT_RF_I2V_BYPASS/dr bit1 0;bit0 1;
    {0x002c, 0x660F}, // i2v_tx_mode		bit14 1;
    {0x0160, 0x00A0}, // rf_sw_rx_en/dr bit6:5 01
    {0x000f, 0x0730}, // bt_sw_tx_en/dr bit9:8 11; adc_pu lna_pd bit11 0; bit10 1; bit5 1; bit4 1;
    {0x0011, 0x4100}, // rx mixer pd		bit9 0; bit8 1;
    // adc config
    {0x03E0, 0x0C60}, // adc corner
    {0x002C, 0x660F}, // rx gain
    {0x03DE, 0x0086}, // adc op1 stb
    {0x03E1, 0x0180}, // adc inres
    {0x0407, 0x0245}, // adc vin sel
    {0x002B, 0x1030}, // adc rstb
    {0x0145, 0xC328}, // adc_sel_dac_ic bit9:6 1100
    {0x0160, 0x00A0}, // adc_sel_dac_ic_dr bit7 1
    {0x0101, 0x00b9}, // adc_att_sel_dr bit5 1
    {0x0103, 0x0000}, // adc_att_sel bit15:9 0
    // mdll clk sel

    {0x0501, 0x0010}, // mdll clkadc en
    // iqcal config
    {0x009D, 0x1F04}, // vres_div2
};

const uint16_t dc_iq_rf_set_t0[][2] =
{
    {0x00c1, 0x3802}, // adc sel clk 1
    {0x009b, 0x7705}, // rfvco_ldo_vres bit14:12 111;
    {0x0093, 0x7ffb}, // iqcal pu bit14:0
    {0x0094, 0x28bf}, // iqcal gain	bit13:0
    {0x00cf, 0x4a00}, // pa_sel_cap_de bit14 1
    {0x00cd, 0xf000}, // pa_sel_cap bit15:12 1111;
    // trxon config
};

const uint16_t dc_iq_rf_set_t1[][2] =
{
    {0x00c1, 0x3f02},//tx ldo_vres 111,和1503先保持一致
    {0x009b, 0x3705},//rx ldo_vres 011
    {0x00c3, 0x0500},//div2_m rx
    {0x004e, 0x1200},//div2_s rx
    {0x0029, 0x881c},//txbuf
    {0x0026, 0x70d0},//rxbuf
    {0x00c1, 0x3f03},//adcclk
    {0x0093, 0x0183}, // iqcal pu bit14:0
    {0x0094, 0x2380}, // iqcal gain	bit13:0
};

const uint32_t dc_iq_dig_store[] =
{
    BT_BES_TESTMODE_ADDR,
    BESMDM_ONE_TONE_SET_GAIN_ADDR,
    BESMDM_ONE_TONE_SET_FREQ_ADDR,
    CMU_REG_58_ADDR,
    CMU_REG_60_ADDR,
    CMU_REG_64_ADDR,
    BESMDM_MIX_MODE_CONFIG_ADDR,
    BESMDM_ONE_TONE_ENERGY_GET_I_ADDR,
    BESMDM_ONE_TONE_ENERGY_GET_Q_ADDR,
};

void btdrv_tx_iq_manual_cal(void)
{
    uint32_t rf_store_tbl_size = ARRAY_SIZE(dc_iq_rf_set_common);
    uint16_t dciq_rf_local[rf_store_tbl_size];

    const uint16_t(*dciq_rf_set_p)[2];
    dciq_rf_set_p = &dc_iq_rf_set_common[0];
    uint32_t dciq_set_tbl_size = ARRAY_SIZE(dc_iq_rf_set_common);

    uint32_t dig_store_tbl_size = ARRAY_SIZE(dc_iq_dig_store);
    uint32_t dciq_dig_local[dig_store_tbl_size];

    uint16_t value = 0;
    uint32_t dig_value = 0;

    uint32_t rf_store_tbl_diff_size = 0;

    const uint16_t(*dciq_rf_set_diff_p)[2];
    uint32_t dciq_set_tbl_diff_size = 0;

    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();

    if (metal_id <= HAL_CHIP_METAL_ID_2) {
        dciq_rf_set_diff_p = &dc_iq_rf_set_t0[0];
        dciq_set_tbl_diff_size = ARRAY_SIZE(dc_iq_rf_set_t0);
        rf_store_tbl_diff_size  = ARRAY_SIZE(dc_iq_rf_set_t0);
    } else if (metal_id >= HAL_CHIP_METAL_ID_3) {
        dciq_rf_set_diff_p = &dc_iq_rf_set_t1[0];
        dciq_set_tbl_diff_size = ARRAY_SIZE(dc_iq_rf_set_t1);
        rf_store_tbl_diff_size = ARRAY_SIZE(dc_iq_rf_set_t1);
    }
    uint16_t dciq_rf_local_diff[rf_store_tbl_diff_size];

    // rf reg store common
    for (uint32_t i = 0; i < rf_store_tbl_size; i++) {
        btdrv_read_rf_reg(dc_iq_rf_set_common[i][0], &value);
        dciq_rf_local[i] = value;
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", dc_iq_rf_set_common[i][0], value);
    }

    // rf reg set common
    for (uint32_t i = 0; i < dciq_set_tbl_size; i++) {
        btdrv_write_rf_reg(dciq_rf_set_p[i][0], dciq_rf_set_p[i][1]);
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", dciq_rf_set_p[i][0], dciq_rf_set_p[i][1]);
    }

    // rf reg store diff
    for (uint32_t i = 0; i < rf_store_tbl_diff_size; i++) {
        btdrv_read_rf_reg(dciq_rf_set_diff_p[i][0], &value);
        dciq_rf_local_diff[i] = value;
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", dc_iq_rf_set_common[i][0], value);
    }

    // rf reg set diff
    for (uint32_t i = 0; i < dciq_set_tbl_diff_size; i++) {
        btdrv_write_rf_reg(dciq_rf_set_diff_p[i][0], dciq_rf_set_diff_p[i][1]);
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", dciq_rf_set_p[i][0], dciq_rf_set_p[i][1]);
    }

    // dig reg store
    for (uint32_t i = 0; i < dig_store_tbl_size; i++) {
        dig_value = BTDIGITAL_REG(dc_iq_dig_store[i]);
        dciq_dig_local[i] = dig_value;
        // BT_DRV_TRACE(2, "dig reg = %x,v = %x", dc_iq_dig_store[i], dig_value);
    }

    dc_iq_calib_1503();

    // rf reg restore common
    for (uint32_t i = 0; i < rf_store_tbl_size; i++) {
        btdrv_write_rf_reg(dc_iq_rf_set_common[i][0], dciq_rf_local[i]);
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", dc_iq_rf_set_common[i][0], dciq_rf_local[i]);
    }

    // dig reg restore common
    for (uint32_t i = 0; i < dig_store_tbl_size; i++) {
        BTDIGITAL_REG_WR(dc_iq_dig_store[i], dciq_dig_local[i]);
        // BT_DRV_TRACE(2, "dig reg = %x,v = %x", dc_iq_dig_store[i], dciq_dig_local[i]);
    }

    if (metal_id <= HAL_CHIP_METAL_ID_2) {
        // rf reg restore common
        for (uint32_t i = 0; i < rf_store_tbl_diff_size; i++) {
            btdrv_write_rf_reg(dc_iq_rf_set_t0[i][0], dciq_rf_local_diff[i]);
            // BT_DRV_TRACE(2, "rf reg = %x,v = %x", dc_iq_rf_set_common[i][0], dciq_rf_local[i]);
        }
    } else if (metal_id >= HAL_CHIP_METAL_ID_3) {
        // rf reg restore common
        for (uint32_t i = 0; i < rf_store_tbl_diff_size; i++) {
            btdrv_write_rf_reg(dc_iq_rf_set_t1[i][0], dciq_rf_local_diff[i]);
            // BT_DRV_TRACE(2, "rf reg = %x,v = %x", dc_iq_rf_set_common[i][0], dciq_rf_local[i]);
        }
    }
}

const uint16_t rx_dccal_rf_set[][2] =
{
    {0x0030,0x0081},
    {0x015e,0x0002},
    {0x0051,0x24F0},
    {0x00d4,0x8001},
    {0x00bf,0x0034},
    {0x002c,0x240E},
    {0x0024,0x0000},
#ifdef BT_RF_I2V_BYPASS
    {0x0309,0x0103},
#else
    {0x0209,0x8112},
#endif
    {0x002A,0x0018},
};

void btdrv_rx_dccal(void)
{
    uint32_t rf_store_tbl_size = ARRAY_SIZE(rx_dccal_rf_set);
    uint16_t value = 0;
    uint16_t dccal_rf_local[rf_store_tbl_size];

    const uint16_t (*dccal_rf_set_p)[2];
    dccal_rf_set_p = &rx_dccal_rf_set[0];

    // rf reg store
    for(uint32_t i = 0; i < rf_store_tbl_size; i++) {
        btdrv_read_rf_reg(rx_dccal_rf_set[i][0], &value);
        dccal_rf_local[i] = value;
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", rx_dccal_rf_set[i][0], value);
    }

    // rf reg set
    for(uint32_t i = 0; i < rf_store_tbl_size; i++) {
        btdrv_write_rf_reg(dccal_rf_set_p[i][0], dccal_rf_set_p[i][1]);
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", dccal_rf_set_p[i][0], dccal_rf_set_p[i][1]);
    }

    rx_dc_cal_1503();

    // rf reg restore
    for(uint32_t i = 0; i < rf_store_tbl_size; i++) {
        btdrv_write_rf_reg(rx_dccal_rf_set[i][0], dccal_rf_local[i]);
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", rx_dccal_rf_set[i][0], dccal_rf_local[i]);
    }
}

#ifndef BT_RF_I2V_BYPASS
struct RX_RCCAL_CALIB
{
    uint16_t rf_reg;
    uint16_t mask;
    uint16_t shift;
};

static struct RX_RCCAL_CALIB RX_RCCAL_CALIB_RF_REG_TBL[] =
{
    {0x134, 0xFF, 8},
    {0x135, 0xFF, 8},
    {0x136, 0xFF, 8},
    {0x137, 0xFF, 8},
    {0x138, 0xFF, 8},
    {0x139, 0xFF, 8},
    {0x13a, 0xFF, 8},
    {0x13b, 0xFF, 8},    // bit[15:8] //i2v corner sw agc tab1
    {0x23f, 0xFF, 8},
    {0x242, 0xFF, 8},
    {0x245, 0xFF, 8},
    {0x248, 0xFF, 8},
    {0x24b, 0xFF, 8},
    {0x24e, 0xFF, 8},
    {0x251, 0xFF, 8},
    {0x254, 0xFF, 8},    // bit[15:8] //i2v corner hw agc tab1
    {0x3e0, 0xFF, 0},    // bit[7:0] //adc corner 1M
    {0x3df, 0xFF, 8},    // bit[15:8] //adc corner 2M
    {0x3df, 0xFF, 0},    // bit[7:0] //adc corner 4M
};

void btdrv_rxbb_rccal(void)
{
    uint16_t value = 0;
    uint16_t rccal_count = 0;
    const uint16_t base_count = 0x1EB5;
    float quotient = 0.0f;
    float factor;
    uint16_t read_val;
    uint8_t i;

    uint16_t rf_store_439;
    uint16_t rf_store_407;
    uint16_t rf_store_525;
    uint32_t dig_store;

    dig_store = BTDIGITAL_REG(BT_BES_TESTMODE_ADDR);
    btdrv_read_rf_reg(0x439, &rf_store_439);
    btdrv_read_rf_reg(0x407, &rf_store_407);
    btdrv_read_rf_reg(0x525, &rf_store_525);

    BTDIGITAL_REG_WR(BT_BES_TESTMODE_ADDR, 0x000A0080);       //rx on

    BTRF_REG_SET_FIELD(0x439, 0x1, 4, 0x1);           //reg_rcosc_pu
    BTRF_REG_SET_FIELD(0x407, 0x3, 3, 0x3);           //reg_rccal_en
    BTRF_REG_SET_FIELD(0x525, 0x1, 0, 0x0);           //reset=0
    BTRF_REG_SET_FIELD(0x525, 0x1, 0, 0x1);           //reset=1
    btdrv_delay(1);

    btdrv_read_rf_reg(0x406, &value);
    BT_DRV_TRACE(2, "%s rf_406=0x%x", __func__, value);
    //bit12: dig_rccal_finish
    if (getbit(value, 15)) {
        rccal_count = value & 0x7FFF;                //dig_rccal_count[11:0]
        BT_DRV_TRACE(2, "%s rccal_count=0x%x", __func__, rccal_count);
    } else {
        BT_DRV_TRACE(1, "%s dig rccal not finish!!!", __func__);
        goto exit;
    }

    quotient = (float)((float)rccal_count / (float)base_count);
    //TRACE(1, "quotient=%f", (double)quotient);
    if ((quotient > 0.7f) && (quotient < 1.3f)) {
        factor = quotient;
    } else {
        // use efuse calib
        BT_DRV_TRACE(0, "quotient not in range [0.7, 1.3]");
        factor = 1.0f;
    }

    for (i = 0; i < ARRAY_SIZE(RX_RCCAL_CALIB_RF_REG_TBL); i++){
        BTRF_REG_GET_FIELD(RX_RCCAL_CALIB_RF_REG_TBL[i].rf_reg, RX_RCCAL_CALIB_RF_REG_TBL[i].mask,
            RX_RCCAL_CALIB_RF_REG_TBL[i].shift, read_val);
        read_val = (uint16_t)((read_val / factor) + 0.5);
        read_val &= 0xFF;
        BTRF_REG_SET_FIELD(RX_RCCAL_CALIB_RF_REG_TBL[i].rf_reg, RX_RCCAL_CALIB_RF_REG_TBL[i].mask,
            RX_RCCAL_CALIB_RF_REG_TBL[i].shift, read_val);
    }

exit:
    //reset
    BTDIGITAL_REG_WR(BT_BES_TESTMODE_ADDR, dig_store);
    btdrv_write_rf_reg(0x525, rf_store_525);
    btdrv_write_rf_reg(0x407, rf_store_407);
    btdrv_write_rf_reg(0x439, rf_store_439);
}
#endif

void bt_drv_calibration_init(void)
{
    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();

    if (metal_id >= HAL_CHIP_METAL_ID_6) {
        // rf pll aac set default val
        btdrv_txpower_calib();
        btdrv_rf_pll_aac_default_val();
    }
#ifndef BT_RF_I2V_BYPASS
    btdrv_rxbb_rccal();
#endif
#ifdef RX_DC_CAL
    btdrv_rx_dccal();
#endif
#ifdef TX_IQ_CAL
    btdrv_tx_iq_manual_cal();
#endif
}

#ifndef PROGRAMMER
static void btdrv_temperature_pa_current_comp(uint32_t pa_current_delta)
{
    static bool initial = false;
    static uint16_t bt_pa_current[8];
    static uint16_t ble_pa_current[8];
    uint16_t bt_pa_current_final = 0;
    uint16_t ble_pa_current_final = 0;

    if (!initial) {
        btdrv_regop_temp_calib_get_tx_gain_table_value_v2(bt_pa_current, ble_pa_current);
        initial = true;
    }

    for (uint8_t i = 0; i < 8; i++) {
        bt_pa_current_final = bt_pa_current[i] + pa_current_delta;
        ble_pa_current_final = ble_pa_current[i] + pa_current_delta;

        if (bt_pa_current_final > 0x1F) {
            bt_pa_current_final = 0x1F;
        }
        if (ble_pa_current_final > 0x1F) {
            ble_pa_current_final = 0x1F;
        }
        btdrv_regop_temp_calib_set_tx_gain_table_value_v2(i, bt_pa_current_final, ble_pa_current_final);
    }
}

static void compensate_irqhandler(uint16_t raw, HAL_GPADC_MV_T volt)
{
    uint16_t initial_temperature = 25;
    static bool initial = false;
    int16_t delta_temp = 0;
    int current_temp = 0;
    static uint16_t tmx_gain_initial = 0;
    uint16_t tmx_gain_final = 0;
    static uint16_t pa_current_initial = 0;
    uint16_t pa_current_final = 0;
    uint16_t pa_current_delta = 0;
    static uint16_t dig_gain_initial = 512;
    uint16_t dig_gain = 0;
    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();

    current_temp = pmu_volt2temperature(volt);

    if (!initial) {
        btdrv_regop_temperature_calib_get_initial_val(&tmx_gain_initial, &pa_current_initial, &dig_gain_initial);
        initial = true;
    }

    delta_temp = current_temp - initial_temperature;
    dig_gain = dig_gain_initial;
    tmx_gain_final = tmx_gain_initial;
    pa_current_final = pa_current_initial;

    if (metal_id >= HAL_CHIP_METAL_ID_6) {
        if (delta_temp < 0) {
            dig_gain = (uint16_t)((float)dig_gain_initial * (float)pow(10.0, ((float)delta_temp/580.0)));
        } else {
            pa_current_delta = (uint16_t)((float)delta_temp/5 - 1);
        }
    } else {
        if (delta_temp < 0) {
            dig_gain = (uint16_t)((float)dig_gain_initial * (float)pow(10.0, ((float)delta_temp/900.0)));
        } else if ((delta_temp > 15) && (delta_temp <= 35)) {
            tmx_gain_final = 0xF;
        } else if (delta_temp > 35){
            tmx_gain_final = 0xF;
            pa_current_final= 0x1F;
        }
    }
    BT_DRV_TRACE(0,"temperature:%d, dig_gain:0x%x", current_temp, dig_gain);

    btdrv_regop_tx_power_dig_comp(dig_gain);
    BTRF_REG_SET_FIELD(0x191, 0xF, 0, tmx_gain_final);
    if (metal_id >= HAL_CHIP_METAL_ID_6) {
        btdrv_temperature_pa_current_comp(pa_current_delta);
    } else {
        btdrv_regop_temp_calib_set_calib_value_v1(tmx_gain_final, pa_current_final);
    }
}

static POSSIBLY_UNUSED void btdrv_compensate_power_based_on_temperature(void)
{
    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_ONESHOT, compensate_irqhandler);
}
#endif

#ifdef BT_RF_MODEM_REG_DUMP
void bt_drv_dump_rf_reg(void)
{
    uint16_t read_val = 0;

    // page1
    for (uint16_t i = 0; i <= 251; i++) {
        btdrv_read_rf_reg(i, &read_val);
        BT_DRV_TRACE(0,"0x%04x: 0x%04x", i, read_val);
        btdrv_delay(1);
    }

    // page2
    for (uint16_t i = 0; i <= 252; i++) {
        btdrv_read_rf_reg((0x100 + i), &read_val);
        BT_DRV_TRACE(0,"0x%04x: 0x%04x", (0x100 + i), read_val);
        btdrv_delay(1);
    }

    // page3
    for (uint16_t i = 0; i <= 221; i++) {
        btdrv_read_rf_reg((0x200 + i), &read_val);
        BT_DRV_TRACE(0,"0x%04x: 0x%04x", (0x200 + i), read_val);
        btdrv_delay(1);
    }

    // page4
    for (uint16_t i = 0; i <= 226; i++) {
        btdrv_read_rf_reg((0x300 + i), &read_val);
        BT_DRV_TRACE(0,"0x%04x: 0x%04x", (0x300 + i), read_val);
        btdrv_delay(1);
    }

    // page5
    for (uint16_t i = 0; i <= 57; i++) {
        btdrv_read_rf_reg((0x400 + i), &read_val);
        BT_DRV_TRACE(0,"0x%04x: 0x%04x", (0x400 + i), read_val);
        btdrv_delay(1);
    }
}

static void bt_drv_dump_modem_reg(void)
{
    // d0350000 ~ d0350600
    uint32_t read_val = 0;

    for (uint16_t i = 0; i <= 384; i++) {
        read_val = BTDIGITAL_REG((CFG_MODEM_BASE_ADDR + (i*4)));
        BT_DRV_TRACE(0,"0x%08x: 0x%08x", (CFG_MODEM_BASE_ADDR + (i*4)), read_val);
        btdrv_delay(1);
    }
}

static void bt_drv_log_power_off_reg_compare(void)
{
    BT_DRV_TRACE(0, "dump rf reg and modem reg before bt sleep.");
    bt_drv_dump_rf_reg();
    bt_drv_dump_modem_reg();

    // sleep
    btdrv_hciopen();
    btdrv_sleep_config(1);
    osDelay(500);
    btdrv_sleep_config(0);
    btdrv_hcioff();

    BT_DRV_TRACE(0, "dump rf reg and modem reg after bt sleep.");
    bt_drv_dump_rf_reg();
    bt_drv_dump_modem_reg();
}
#endif

void btdrv_start_bt(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_52M);

    btdrv_common_init();

    hal_iomux_ispi_access_enable(HAL_IOMUX_ISPI_MCU_RF);

    // Symbol init should be done before power on BTC
    bt_drv_reg_op_global_symbols_init();
    /*reg controller crash dump*/
    hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_BT, btdrv_btc_fault_dump);

#ifdef __BT_RAMRUN_NEW__
    bt_controller_open(NULL, NULL);
    BT_DRV_TRACE(1, "bt ramrun %s", BT_CONTROLLER_COMMIT_ID);
    BT_DRV_TRACE(1, "bt ramrun %s", BT_CONTROLLER_COMMIT_DATE);
    btdrv_delay(100);
#else
    // power on BT CPU
    btdrv_poweron(BT_POWERON);
    // load BTC patch
    btdrv_ins_patch_init();
#endif
#ifdef BTC_CPUDUMP_BASE
    btdrv_cpudump_clk_enable();
    btdrv_cpudump_enable();
#endif
#ifdef MCU_WAKEUP_BT_V2
    hal_intersys_open(HAL_INTERSYS_ID_1, HAL_INTERSYS_MSG_HCI, NULL, NULL, false);
#endif //MCU_WAKEUP_BT_V2

#ifndef BT_PLD_SIMU
    // RF module init
    btdrv_rf_init();
    // Digital module init
    btdrv_digital_init();
    // calibration module init
    bt_drv_calibration_init();
#endif

#ifndef PROGRAMMER
    btdrv_register_temp_compensation_callback(btdrv_compensate_power_based_on_temperature);
#endif
    //write BTC srand seed
    btdrv_regop_set_btc_srand_seed_initial();

    // config BTC default setting through HCI
    btdrv_hciopen();
    btdrv_hciprocess();
    btdrv_hcioff();

    // config BTC default setting by writing BTC SRAM directly
    bt_drv_reg_op_init();

#ifdef BT_RF_MODEM_REG_DUMP
    bt_drv_log_power_off_reg_compare();
#endif

    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_32K);
}

const uint8_t hci_cmd_enable_dut[] =
{
    0x01, 0x03, 0x18, 0x00
};
const uint8_t hci_cmd_enable_allscan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x03
};
const uint8_t hci_cmd_disable_scan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x00
};
const uint8_t hci_cmd_enable_pagescan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x02
};
const uint8_t hci_cmd_autoaccept_connect[] =
{
    0x01, 0x05, 0x0c, 0x03, 0x02, 0x00, 0x02
};
const uint8_t hci_cmd_hci_reset[] =
{
    0x01, 0x03, 0x0c, 0x00
};

const uint8_t hci_cmd_inquiry_scan_activity[] =
{
    0x01, 0x1e, 0x0c, 0x04, 0x40, 0x00, 0x12, 0x00
};

const uint8_t hci_cmd_page_scan_activity[] =
{
    0x01, 0x1c, 0x0c, 0x04, 0x40, 0x00, 0x12, 0x00
};

const uint8_t hci_cmd_nonsig_tx_dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x00, 0x04, 0x04, 0x1b, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_2dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x04, 0x04, 0x36, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_3dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x08, 0x04, 0x53, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_2dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0a, 0x04, 0x6f, 0x01,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_3dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0b, 0x04, 0x28, 0x02,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};

const uint8_t hci_cmd_nonsig_rx_dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x00, 0x04, 0x00, 0x1b, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_2dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x04, 0x00, 0x36, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_3dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x08, 0x00, 0x53, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_2dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0a, 0x00, 0x6f, 0x01,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_3dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0b, 0x00, 0x28, 0x02,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};

// vco test
const uint8_t hci_cmd_start_bt_vco_test[] =
{
    0x01, 0xaa, 0xfc, 0x02, 0x00, 0x02
};
const uint8_t hci_cmd_stop_bt_vco_test[] =
{
    0x01, 0xaa, 0xfc, 0x02, 0x00, 0x04
};

const uint8_t hci_cmd_enable_ibrt_test[] =
{
    0x01, 0xb4, 0xfc, 0x01, 0x01
};

const uint8_t hci_cmd_set_ibrt_mode[] =
{
    0x01, 0xa2, 0xfc, 0x02, 0x01, 0x00
};

void btdrv_testmode_start(void)
{
}

void btdrv_testmode_end(void)
{
}

void btdrv_write_localinfo(const char *name, uint8_t len, const uint8_t *addr)
{
    uint8_t hci_cmd_write_addr[5+6] =
    {
        0x01, 0x72, 0xfc, 0x07, 0x00
    };

    uint8_t hci_cmd_write_name[248+4] =
    {
        0x01, 0x13, 0x0c, 0xF8
    };
    memset(&hci_cmd_write_name[4], 0, sizeof(hci_cmd_write_name) - 4);
    memcpy(&hci_cmd_write_name[4], name, len);
    btdrv_SendData(hci_cmd_write_name, sizeof(hci_cmd_write_name));
    btdrv_delay(50);
    memcpy(&hci_cmd_write_addr[5], addr, 6);
    btdrv_SendData(hci_cmd_write_addr, sizeof(hci_cmd_write_addr));
    btdrv_delay(20);
}

void btdrv_enable_dut(void)
{
    btdrv_SendData(hci_cmd_enable_dut, sizeof(hci_cmd_enable_dut));
    btdrv_delay(100);
    btdrv_SendData(hci_cmd_enable_allscan, sizeof(hci_cmd_enable_allscan));
    btdrv_delay(100);
    btdrv_SendData(hci_cmd_autoaccept_connect, sizeof(hci_cmd_autoaccept_connect));
    btdrv_delay(100);
    btdrv_dut_mode_enable = true;
}

void btdrv_enable_dut_again(void)
{
    btdrv_SendData(hci_cmd_enable_allscan, sizeof(hci_cmd_enable_allscan));
    btdrv_delay(20);
    btdrv_dut_mode_enable = true;
}

void btdrv_enable_ibrt_test(void)
{
    btdrv_SendData(hci_cmd_enable_ibrt_test, sizeof(hci_cmd_enable_ibrt_test));
    btdrv_delay(20);
    btdrv_SendData(hci_cmd_set_ibrt_mode, sizeof(hci_cmd_set_ibrt_mode));
    btdrv_delay(20);
}

void btdrv_connect_ibrt_device(uint8_t *addr)
{
    uint8_t hci_cmd_connect_device[17] =
    {
        0x01, 0x05, 0x04, 0x0D, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0x18, 0xCC,
        0x01, 0x00, 0x00, 0x00, 0x00
    };
    memcpy(&hci_cmd_connect_device[4], addr, 6);
    btdrv_SendData(hci_cmd_connect_device, sizeof(hci_cmd_connect_device));
    btdrv_delay(50);
}

void btdrv_disable_scan(void)
{
    btdrv_SendData(hci_cmd_disable_scan, sizeof(hci_cmd_disable_scan));
    btdrv_delay(20);
}

static uint32_t dut_connect_status = DUT_CONNECT_STATUS_DISCONNECTED;

uint32_t btdrv_dut_get_connect_status(void)
{
    return dut_connect_status;
}

void btdrv_enable_dut_again(void);
void btdrv_dut_accessible_mode_manager(const unsigned char *data)
{
    if (btdrv_dut_mode_enable)
    {
        if (data[0] == 0x04 && data[1] == 0x03 && data[2] == 0x0b && data[3] == 0x00)
        {
#ifdef __IBRT_IBRT_TESTMODE__
            if (memcmp(&data[6], bt_addr, 6))
            {
                btdrv_disable_scan();
            }
#else
            btdrv_disable_scan();
#endif
            dut_connect_status = DUT_CONNECT_STATUS_CONNECTED;
        }
        else if (data[0] == 0x04 && data[1] == 0x05 && data[2] == 0x04 && data[3] == 0x00)
        {
            btdrv_enable_dut_again();
            dut_connect_status = DUT_CONNECT_STATUS_DISCONNECTED;
        }
    }
}

void btdrv_feature_default(void)
{
#ifdef __EBQ_TEST__
    const uint8_t feature[] = {0xBF, 0xFE, 0x4F, 0xFe, 0xdb, 0xFF, 0x5b, 0x87};
#else
    const uint8_t feature[] = {0xBF, 0xeE, 0x4D, 0xFe, 0xdb, 0xFf, 0x7b, 0x87};
#endif
    btdrv_send_cmd(HCI_DBG_SET_LOCAL_FEATURE_CMD_OPCODE, 8, feature);
    btdrv_delay(1);
}

void btdrv_hci_reset(void)
{
    btdrv_SendData(hci_cmd_hci_reset, sizeof(hci_cmd_hci_reset));
    btdrv_delay(350);
}

void btdrv_enable_nonsig_tx(uint8_t index)
{
    BT_DRV_TRACE(1, "%s\n", __func__);

    if (index == 0)
        btdrv_SendData(hci_cmd_nonsig_tx_2dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_2dh1_pn9_t0));
    else if (index == 1)
        btdrv_SendData(hci_cmd_nonsig_tx_3dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_3dh1_pn9_t0));
    else if (index == 2)
        btdrv_SendData(hci_cmd_nonsig_tx_2dh3_pn9_t0, sizeof(hci_cmd_nonsig_tx_2dh1_pn9_t0));
    else if (index == 3)
        btdrv_SendData(hci_cmd_nonsig_tx_3dh3_pn9_t0, sizeof(hci_cmd_nonsig_tx_3dh1_pn9_t0));
    else
        btdrv_SendData(hci_cmd_nonsig_tx_dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_dh1_pn9_t0));

    btdrv_delay(20);
}

void btdrv_enable_nonsig_rx(uint8_t index)
{
    BT_DRV_TRACE(1, "%s\n", __func__);

    if (index == 0)
        btdrv_SendData(hci_cmd_nonsig_rx_2dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_2dh1_pn9_t0));
    else if (index == 1)
        btdrv_SendData(hci_cmd_nonsig_rx_3dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_3dh1_pn9_t0));
    else if (index == 2)
        btdrv_SendData(hci_cmd_nonsig_rx_2dh3_pn9_t0, sizeof(hci_cmd_nonsig_rx_2dh1_pn9_t0));
    else if (index == 3)
        btdrv_SendData(hci_cmd_nonsig_rx_3dh3_pn9_t0, sizeof(hci_cmd_nonsig_rx_3dh1_pn9_t0));
    else
        btdrv_SendData(hci_cmd_nonsig_rx_dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_dh1_pn9_t0));

    btdrv_delay(20);
}

static bool btdrv_vco_test_running = false;
static uint32_t gfsk_dig_gain = 0;
static uint32_t dc_gain = 0;
static uint16_t vco_test_reg_val_24 = 0;

void btdrv_vco_test_start(uint8_t chnl)
{
    if (!btdrv_vco_test_running)
    {
        btdrv_vco_test_running = true;
        gfsk_dig_gain = BTDIGITAL_REG(BESMDM_GFSK_DSG_NOM_ADDR);
        dc_gain = BTDIGITAL_REG(BESMDM_IQMCNTL_ADDR_CT_ADDR);
        btdrv_read_rf_reg(0x24, &vco_test_reg_val_24);

        btdrv_reg_op_vco_test_start(chnl);

        // Transmit signal without modulation
        besmdm_gfsk_dsg_nom_pack(0x0, 0x0);
        besmdm_iqmcntl_addr_ct_pack(0x0,0x0,0x0,0x340,0x340);
        BTRF_REG_SET_FIELD(0x24, 0xF, 4, 7);     //tx gain dr idx3
    }
}

void btdrv_vco_test_stop(void)
{
    if (btdrv_vco_test_running)
    {
        btdrv_vco_test_running = false;
        btdrv_reg_op_vco_test_stop();

        BTDIGITAL_REG_WR(BESMDM_GFSK_DSG_NOM_ADDR, gfsk_dig_gain);
        BTDIGITAL_REG_WR(BESMDM_IQMCNTL_ADDR_CT_ADDR, dc_gain);
        btdrv_write_rf_reg(0x24, vco_test_reg_val_24);
    }
}

void btdrv_stop_bt(void)
{
    /*unregister BT crash dump*/
    hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_BT, NULL);

    btdrv_poweron(BT_POWEROFF);
}

void btdrv_write_memory(uint8_t wr_type, uint32_t address, const uint8_t *value, uint8_t length)
{
    uint8_t buff[256];
    if (length == 0 || length > 128)
        return;
    buff[0] = 0x01;
    buff[1] = 0x02;
    buff[2] = 0xfc;
    buff[3] = length + 6;
    buff[4] = address & 0xff;
    buff[5] = (address & 0xff00) >> 8;
    buff[6] = (address & 0xff0000) >> 16;
    buff[7] = address >> 24;
    buff[8] = wr_type;
    buff[9] = length;
    memcpy(&buff[10], value, length);
    btdrv_SendData(buff, length + 10);
    btdrv_delay(2);
}

void btdrv_send_cmd(uint16_t opcode, uint8_t cmdlen, const uint8_t *param)
{
    uint8_t buff[256];
    buff[0] = 0x01;
    buff[1] = opcode & 0xff;
    buff[2] = (opcode & 0xff00) >> 8;
    buff[3] = cmdlen;
    if (cmdlen > 0)
        memcpy(&buff[4], param, cmdlen);
    btdrv_SendData(buff, cmdlen + 4);
}


uint32_t btdrv_syn_get_curr_ticks(void)
{
    return btdrv_reg_op_syn_get_curr_ticks();
}

uint32_t btdrv_syn_get_cis_curr_time(void)
{
    return btdrv_reg_op_syn_get_cis_curr_time();
}

void bt_syn_trig_checker(uint16_t conhdl)
{
}

// Can be used by master or slave
// Used for bt clk calculate to bts
// Hus must read from hw reg, should not use btc sw saved hus
uint32_t bt_syn_ble_bt_time_to_bts(uint32_t hs, uint16_t hus)
{
    return btdrv_reg_op_bt_time_to_bts(hs, HALF_SLOT_INV(hus));
}

void btdrv_enable_playback_triggler(uint8_t triggle_mode)
{
    btdrv_reg_op_enable_playback_triggler(triggle_mode);
}

void btdrv_disable_playback_triggler(void)
{
    btdrv_reg_op_disable_playback_triggler();
}

// pealse use btdrv_is_link_index_valid() check link index whether valid
uint8_t btdrv_conhdl_to_linkid(uint16_t connect_hdl)
{
    // invalid hci handle,such as link disconnected
    if (connect_hdl < HCI_HANDLE_MIN || connect_hdl > HCI_HANDLE_MAX)
    {
        TRACE(2, "ERROR Connect Handle=0x%x ca=%p", connect_hdl, __builtin_return_address(0));
        return HCI_LINK_INDEX_INVALID;
    }
    else
    {
        return (connect_hdl - HCI_HANDLE_MIN);
    }
}

// Trace tport
static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_tport[] =
{
    {HAL_IOMUX_PIN_P0_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
};

int btdrv_host_gpio_tport_open(void)
{
    uint32_t i;

    for (i = 0; i < ARRAY_SIZE(pinmux_tport); i++)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&pinmux_tport[i], 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pinmux_tport[i].pin, HAL_GPIO_DIR_OUT, 0);
    }
    return 0;
}

int btdrv_gpio_port_set(int port)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)pinmux_tport[port].pin);
    return 0;
}

int btdrv_gpio_tport_clr(int port)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)pinmux_tport[port].pin);
    return 0;
}

#ifdef PCM_FAST_MODE
void btdrv_open_pcm_fast_mode_enable(void)
{
    btdrv_reg_op_open_pcm_fast_mode_enable();
}
void btdrv_open_pcm_fast_mode_disable(void)
{
    btdrv_reg_op_open_pcm_fast_mode_disable();
}
#endif

#if defined(CVSD_BYPASS)
void btdrv_cvsd_bypass_enable(void)
{
    btdrv_reg_op_cvsd_bypass_enable();
}
#endif

void btdrv_enable_rf_sw(int rx_on, int tx_on)
{
    hal_iomux_set_bt_rf_sw(rx_on, tx_on);
}
