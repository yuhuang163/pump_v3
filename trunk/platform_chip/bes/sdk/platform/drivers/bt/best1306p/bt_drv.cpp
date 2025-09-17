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


#ifdef DPD_ONCHIP_CAL
#include "bt_drv_dpd_mem.h"
#endif
// CHIP related
#include "bt_drv_1306p_internal.h"
#include "bt_drv_1306p_config.h"
#include "bt_1306p_reg_map.h"
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
    pmu_bt_2v5pa(en);
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

    {0x040b, 0x0301}, // pa_short_en dr		bit1 0; bit0 1;
    {0x0160, 0x00A0}, // adc_sel_dac_ic_dr bit7 1
    {0x000F, 0x0730}, // bt_sw_tx_en/dr bit9:8 11; adc_pu lna_pd bit11 0; bit10 1; bit5 1; bit4 1
    {0x0011, 0x4100}, // rx mixer pd		bit9 0; bit8 1;
    {0x002c, 0x660F}, // i2v_tx_mode		bit14 1;
    {0x0160, 0x00A0}, // rf_sw_rx_en/dr bit6:5 01
    {0x0011, 0x4100}, // rx mixer pd		bit9 0; bit8 1;
    {0x00BF, 0x0014}, // rx_i2v_pd
    // adc config
    {0x002C, 0x2807}, // rx gain
    {0x0095, 0x4838}, // adc_int2_cap,adc_vcm, adc_int2_cap,adc_vcm
    {0x00C2, 0x5F38}, // adc corner
    {0x00C1, 0xAD02}, // adc corner
    {0x0091, 0x6180}, // adc op1 stb adc op1 stb dr
    {0x0145, 0xC328}, // adc inres
    {0x0147, 0xC065}, // adc inres dr
    {0x0101, 0x00A0}, // adc att dr
    {0x0407, 0x0205}, // adc_overflow,adc_vin_sel dr
    {0x0409, 0x1600}, // dc vin sel0
    {0x0097, 0x2FF0}, // dvdd
    {0x0099, 0x303B}, // adc_bias5u
    {0x0501, 0x0010}, // mdll_clk
    {0x050D, 0x4000}, // mdll clk
    {0x0093, 0x0183}, // iqcal pu        bit14:0
    {0x0094, 0x3800}, // iqcal gain      bit13:0
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

    // dig reg store
    for (uint32_t i = 0; i < dig_store_tbl_size; i++) {
        dig_value = BTDIGITAL_REG(dc_iq_dig_store[i]);
        dciq_dig_local[i] = dig_value;
        // BT_DRV_TRACE(2, "dig reg = %x,v = %x", dc_iq_dig_store[i], dig_value);
    }

    dc_iq_calib_1306p();

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

}

const uint16_t rx_dccal_rf_set[][2] =
{
    {0x0030,0x0081},
    {0x015e,0x0002},
    {0x0051,0x24F0},
    {0x00d4,0x8001},
    {0x0024,0x0000},
    {0x0209,0x8152},
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

    rx_dc_cal_1306p();

    // rf reg restore
    for(uint32_t i = 0; i < rf_store_tbl_size; i++) {
        btdrv_write_rf_reg(rx_dccal_rf_set[i][0], dccal_rf_local[i]);
        // BT_DRV_TRACE(2, "rf reg = %x,v = %x", rx_dccal_rf_set[i][0], dccal_rf_local[i]);
    }
}

struct RX_RCCAL_CALIB
{
    uint16_t rf_reg;
    uint16_t mask;
    uint16_t shift;
};

static struct RX_RCCAL_CALIB RX_RCCAL_CALIB_RF_REG_TBL[] =
{
    {0x0134, 0xFF, 8},      //i2v corner sw agc tab1
    {0x01D6, 0xFF, 1},      //adc corner sw agc tab1
    {0x023F, 0xFF, 8},      //i2v corner hw agc tab1
    {0x02E6, 0xFF, 1},      //adc corner hw agc tab1
};

void btdrv_rxbb_rccal(void)
{
    uint16_t value = 0;
    uint16_t rccal_count = 0;
    const uint16_t base_count = 0x1DDB;
    float quotient = 0.0f;
    float factor;
    uint16_t read_val;
    uint8_t i;

    uint16_t rf_store_439;
    uint16_t rf_store_407;

    btdrv_read_rf_reg(0x439, &rf_store_439);
    btdrv_read_rf_reg(0x407, &rf_store_407);

    BTRF_REG_SET_FIELD(0x407, 0x7, 5, 0x2);
    btdrv_delay(1);
    BTRF_REG_SET_FIELD(0x439, 0x1, 4, 0x1);           //reg_rcosc_pu
    BTRF_REG_SET_FIELD(0x407, 0x3, 3, 0x3);           //reg_rccal_en
    btdrv_delay(1);

    btdrv_read_rf_reg(0x5A, &value);
    BT_DRV_TRACE(2, "%s rf_5A=0x%x", __func__, value);
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

    for (uint8_t idx = 0; idx < 8; idx++) {
        for (i = 0; i < ARRAY_SIZE(RX_RCCAL_CALIB_RF_REG_TBL); i++){
            if (RX_RCCAL_CALIB_RF_REG_TBL[i].rf_reg == 0x23F) {
                idx *= 3;
            }
            BTRF_REG_GET_FIELD((RX_RCCAL_CALIB_RF_REG_TBL[i].rf_reg + idx), RX_RCCAL_CALIB_RF_REG_TBL[i].mask,
                RX_RCCAL_CALIB_RF_REG_TBL[i].shift, read_val);

            read_val = (uint16_t)((read_val / factor) + 0.5);

            // workround: The lowest 1 bit of i2v_corner is not calibrated, which affects the sensitivity.
            if ((RX_RCCAL_CALIB_RF_REG_TBL[i].rf_reg == 0x23F) || (RX_RCCAL_CALIB_RF_REG_TBL[i].rf_reg == 0x134)) {
                read_val &= 0xFE;
            } else {
                read_val &= 0xFF;
            }

            BTRF_REG_SET_FIELD((RX_RCCAL_CALIB_RF_REG_TBL[i].rf_reg + idx), RX_RCCAL_CALIB_RF_REG_TBL[i].mask,
                RX_RCCAL_CALIB_RF_REG_TBL[i].shift, read_val);
            if (RX_RCCAL_CALIB_RF_REG_TBL[i].rf_reg == 0x23F) {
                idx /= 3;
            }
        }
    }
exit:
    //reset
    btdrv_write_rf_reg(0x407, rf_store_407);
    btdrv_write_rf_reg(0x439, rf_store_439);
}

#ifdef PMU_NTC_MONITOR
#define TXPWR_COMP_DECREASE_1_DBM     2
void btdrv_tx_power_temperature_compensate(int temperature)
{
    static bool tx_power_comp_flag = false;
    static bool initial = false;
    static uint16_t gsg_nom = 0;
    static uint16_t dsg_nom = 0;

    if (!initial) {
        BTDIGITAL_REG_GET_FIELD(BESMDM_GFSK_DSG_NOM_ADDR, 0x3FF, 0, gsg_nom);
        BTDIGITAL_REG_GET_FIELD(BESMDM_DPSK_DSG_NOM_ADDR, 0x3FF, 0, dsg_nom);
        initial = true;
    }

    if ((temperature < 0) && !tx_power_comp_flag) {
        // xtal mankick en
        gsg_nom = gsg_nom - TXPWR_COMP_DECREASE_1_DBM;
        dsg_nom = dsg_nom - TXPWR_COMP_DECREASE_1_DBM;
        BTDIGITAL_REG_SET_FIELD(BESMDM_GFSK_DSG_NOM_ADDR, 0x3FF, 0, gsg_nom);
        BTDIGITAL_REG_SET_FIELD(BESMDM_GFSK_DSG_NOM_ADDR, 0x3FF, 10, gsg_nom);
        BTDIGITAL_REG_SET_FIELD(BESMDM_DPSK_DSG_NOM_ADDR, 0x3FF, 0, dsg_nom);
        tx_power_comp_flag = true;
        BT_DRV_TRACE(0,"temperature:%d < 0, tx power compensate enable", temperature);
    } else if ((temperature >= 5) && tx_power_comp_flag){
        // xtal mankick close
        gsg_nom = gsg_nom + TXPWR_COMP_DECREASE_1_DBM;
        dsg_nom = dsg_nom + TXPWR_COMP_DECREASE_1_DBM;
        BTDIGITAL_REG_SET_FIELD(BESMDM_GFSK_DSG_NOM_ADDR, 0x3FF, 0, gsg_nom);
        BTDIGITAL_REG_SET_FIELD(BESMDM_GFSK_DSG_NOM_ADDR, 0x3FF, 10, gsg_nom);
        BTDIGITAL_REG_SET_FIELD(BESMDM_DPSK_DSG_NOM_ADDR, 0x3FF, 0, dsg_nom);
        tx_power_comp_flag = false;
        BT_DRV_TRACE(0,"temperature:%d >= 5, tx power compensate disable", temperature);
    }
}

static void bt_temperature_comp_handler(void const *param);
osTimerDef(bt_temperature_comp_timer, bt_temperature_comp_handler);
static osTimerId bt_temperature_comp_timer_id = NULL;
static void bt_temperature_comp_handler(void const *param)
{
    POSSIBLY_UNUSED int ret = 0;
    struct PMU_NTC_CTX_T ctx;

    ret = pmu_ntc_ctx_get(&ctx);

    if (!ret) {
        btdrv_tx_power_temperature_compensate(ctx.temperature);

        BT_DRV_TRACE(0,"%s, temperature: %d", __func__, ctx.temperature);
    }
}

static void btdrv_temperature_comp(void)
{
    static struct PMU_NTC_MONITOR_CTX_T pmu_ntc_monitor_ctx;

    pmu_ntc_monitor_ctx.interval_ms = 10000;
    pmu_ntc_monitor_ctx.enable = true;

    pmu_ntc_monitor_register(PMU_NTC_USER_BT, &pmu_ntc_monitor_ctx);

    if (!bt_temperature_comp_timer_id){
        bt_temperature_comp_timer_id = osTimerCreate(osTimer(bt_temperature_comp_timer), osTimerPeriodic, NULL);
    }
    if (bt_temperature_comp_timer_id != NULL) {
        osTimerStart(bt_temperature_comp_timer_id, 10000);
    }
}
#endif

void bt_drv_calibration_init(void)
{
    btdrv_txpower_calib();
    btdrv_rxbb_rccal();

#ifdef RX_DC_CAL
    btdrv_rx_dccal();
#endif

#ifdef TX_IQ_CAL
    btdrv_tx_iq_manual_cal();
#endif

#ifdef PMU_NTC_MONITOR
    btdrv_temperature_comp();
#endif
}

void bt_drv_dump_rf_reg(void)
{
    uint16_t read_val = 0;

    TRACE(0, "%s start", __func__);
    // page1
    for (uint16_t i = 0; i <= 254; i++) {
        btdrv_read_rf_reg(i, &read_val);
        TRACE_IMM(0,"0x%04x: 0x%04x", i, read_val);
        btdrv_delay(1);
    }

    // page2
    for (uint16_t i = 0; i <= 236; i++) {
        btdrv_read_rf_reg((0x100 + i), &read_val);
        TRACE_IMM(0,"0x%04x: 0x%04x", (0x100 + i), read_val);
        btdrv_delay(1);
    }

    // page3
    for (uint16_t i = 0; i <= 246; i++) {
        btdrv_read_rf_reg((0x200 + i), &read_val);
        TRACE_IMM(0,"0x%04x: 0x%04x", (0x200 + i), read_val);
        btdrv_delay(1);
    }

    // page5
    for (uint16_t i = 0; i <= 74; i++) {
        btdrv_read_rf_reg((0x400 + i), &read_val);
        TRACE_IMM(0,"0x%04x: 0x%04x", (0x400 + i), read_val);
        btdrv_delay(1);
    }

    // page6
    for (uint16_t i = 0; i <= 124; i++) {
        btdrv_read_rf_reg((0x500 + i), &read_val);
        TRACE_IMM(0,"0x%04x: 0x%04x", (0x500 + i), read_val);
        btdrv_delay(1);
    }
    TRACE(0, "%s done", __func__);
}

static void POSSIBLY_UNUSED bt_drv_dump_modem_reg(void)
{
    // d0350000 ~ d0350600
    uint32_t read_val = 0;

    for (uint16_t i = 0; i <= 384; i++) {
        read_val = BTDIGITAL_REG((CFG_MODEM_BASE_ADDR + (i << 2)));
        TRACE_IMM(0,"0x%08x: 0x%08x", (CFG_MODEM_BASE_ADDR + (i << 2)), read_val);
        btdrv_delay(1);
    }
}

static void POSSIBLY_UNUSED bt_drv_log_power_off_reg_compare(void)
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

void btdrv_start_bt(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_52M);

    btdrv_common_init();

    hal_iomux_ispi_access_enable(HAL_IOMUX_ISPI_MCU_RF);

    // Symbol init should be done before power on BTC
    bt_drv_reg_op_global_symbols_init();
    /*reg controller crash dump*/
    hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_BT, btdrv_btc_fault_dump);

    // power on BT CPU
    btdrv_poweron(BT_POWERON);
    // load BTC patch
    btdrv_ins_patch_init();

#ifdef BTC_CPUDUMP_BASE
    btdrv_cpudump_clk_enable();
    btdrv_cpudump_enable();
#endif
    // RF module init
    btdrv_rf_init();
    // Digital module init
    btdrv_digital_init();
    // calibration module init
    bt_drv_calibration_init();

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
static uint16_t vco_test_reg_val_9E = 0;
#ifdef VCO_TEST_TOOL
static unsigned short vco_test_hack_flag = 0;
static unsigned short vco_test_channel = 0xff;
static unsigned short set_xatl_facp_hack_flag = 0;
static unsigned short vco_test_xtal_fcap = 0xffff;
static unsigned short pmu_reboot_hci_flag = 0;

unsigned short btdrv_get_vco_test_process_flag(void)
{
    return vco_test_hack_flag;
}

bool btdrv_vco_test_bridge_intsys_callback(const unsigned char *data)
{
    bool status = false;
    if (data[0] == 0x01 && data[1] == 0xaa && data[2] == 0xfc && data[3] == 0x02)
    {
        status = true;
        vco_test_hack_flag = data[5];
        vco_test_channel = data[4];
    }

    return status;
}

void btdrv_vco_test_process(uint8_t op)
{
    if (op == 0x02) // vco test start
    {
        if (vco_test_channel != 0xff)
            btdrv_vco_test_start(vco_test_channel);
    }
    else if (op == 0x04) // vco test stop
    {
        btdrv_vco_test_stop();
    }
    vco_test_channel = 0xff;
    vco_test_hack_flag = 0;
}

unsigned short btdrv_get_vco_test_set_xtal_fcap_process_flag(void)
{
    return set_xatl_facp_hack_flag;
}

bool btdrv_vco_test_set_xtal_fcap_bridge_intsys_callback(const unsigned char *data)
{
    bool status = false;
    if(data[0]==0x01 && data[1]==0xab && data[2]==0xfc && data[3]==0x03)
    {
        status = true;
        vco_test_xtal_fcap = (data[5] << 8) | data[4];
        set_xatl_facp_hack_flag = data[6];
    }

    return status;
}

void btdrv_vco_test_set_xtal_fcap_process(uint8_t op)
{
    if(op == 0x02)// set_xtal_fcap
    {
        btdrv_rf_init_xtal_fcap(vco_test_xtal_fcap);
    }
    else if (op == 0x04)// save_xtal_fcap
    {
        nvrec_dev_set_xtal_fcap(vco_test_xtal_fcap);
    }
    vco_test_xtal_fcap = 0xffff;
    set_xatl_facp_hack_flag = 0;
}

unsigned short btdrv_pmu_reboot_hci_process_flag(void)
{
    return pmu_reboot_hci_flag;
}

bool btdrv_pmu_reboot_hci_intsys_callback(const unsigned char *data)
{
    bool status = false;
    if(data[0]==0x01 && data[1]==0xac && data[2]==0xfc && data[3]==0x01)
    {
        status = true;
        pmu_reboot_hci_flag = data[4];
    }

    return status;
}

void btdrv_pmu_reboot_hci_process(uint8_t op)
{
    if(op == 0x01)
    {
        pmu_reboot();
    }
    pmu_reboot_hci_flag = 0;
}
#endif
void btdrv_vco_test_start(uint8_t chnl)
{
    if (!btdrv_vco_test_running)
    {
        btdrv_vco_test_running = true;
        gfsk_dig_gain = BTDIGITAL_REG(BESMDM_GFSK_DSG_NOM_ADDR);
        dc_gain = BTDIGITAL_REG(BESMDM_IQMCNTL_ADDR_CT_ADDR);
        btdrv_read_rf_reg(0x24, &vco_test_reg_val_24);
        btdrv_read_rf_reg(0x9E, &vco_test_reg_val_9E);

        btdrv_reg_op_vco_test_start(chnl);

        // Transmit signal without modulation
        besmdm_gfsk_dsg_nom_pack(0x0, 0x0);
        besmdm_iqmcntl_addr_ct_pack(0x0,0x0,0x0,0x340,0x340);
        BTRF_REG_SET_FIELD(0x24, 0xF, 4, 7);     //tx gain dr idx3
        BTRF_REG_SET_FIELD(0x9E, 0x1, 7, 0x1);   //reg_bt_rfpll_cp_bypass_r
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
        btdrv_write_rf_reg(0x9E, vco_test_reg_val_9E);
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
