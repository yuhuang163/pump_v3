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
#include <string.h>
#include "pmu.h"
#include "hal_chipid.h"
#ifdef __RF_INIT_XTAL_CAP_FROM_NV__
#include "nvrecord_dev.h"
#endif
#include "bt_drv.h"
#include "bt_drv_interface.h"
#include "bt_drv_internal.h"
#include "bt_drv_reg_op.h"

//CHIP related
#include "bt_drv_1306p_internal.h"
#include "bt_drv_1306p_config.h"
#include CHIP_SPECIFIC_HDR(bt_drv_modem_reg_map)
#define XTAL_FCAP_NORMAL_SHIFT                  0
#define XTAL_FCAP_NORMAL_MASK                   (0x1FF << XTAL_FCAP_NORMAL_SHIFT)
#define XTAL_FCAP_NORMAL(n)                     BITFIELD_VAL(XTAL_FCAP_NORMAL, n)

static uint16_t xtal_fcap = DEFAULT_XTAL_FCAP;
static uint16_t init_xtal_fcap = DEFAULT_XTAL_FCAP;

struct RF_SYS_INIT_ITEM
{
    uint16_t reg;
    uint16_t set;
    uint16_t mask;
    uint16_t delay;
};

struct TX_PWR_SELECT_ITEM
{
    float pwr_dbm;
    uint16_t tx_gain_tbl0;
    uint16_t tx_gain_tbl1;
    uint16_t tx_gain_tbl2;
    uint8_t pa_sel;
};

#define TX_PWR_1P2_PA           (1 << 0)

static const struct TX_PWR_SELECT_ITEM tx_pwr_select[] =
{
    {12,  0x4200, 0x015f, 0x0000, TX_PWR_1P2_PA},   //12dbm
    {11,  0x4200, 0x00f8, 0x0000, TX_PWR_1P2_PA},   //11dbm
    {10,  0x4200, 0x00e3, 0x0000, TX_PWR_1P2_PA},   //10dbm
    {9,   0x4200, 0x00c2, 0x0000, TX_PWR_1P2_PA},   //9dbm
    {8,   0x4200, 0x00e1, 0x0000, TX_PWR_1P2_PA},   //8dbm
    {7,   0x4200, 0x00eb, 0x0001, TX_PWR_1P2_PA},   //7dbm
    {6,   0x4200, 0x00e5, 0x0001, TX_PWR_1P2_PA},   //6dbm
    {5,   0x4200, 0x00d3, 0x0001, TX_PWR_1P2_PA},   //5dbm
    {4,   0x4200, 0x00c2, 0x0001, TX_PWR_1P2_PA},   //4dbm
    {3,   0x4200, 0x00e1, 0x0001, TX_PWR_1P2_PA},   //3dbm
    {2,   0x4200, 0x00e3, 0x0002, TX_PWR_1P2_PA},   //2dbm
    {1,   0x4200, 0x00d2, 0x0002, TX_PWR_1P2_PA},   //1dbm
    {0,   0x4200, 0x00e1, 0x0002, TX_PWR_1P2_PA},   //0dbm
    {-1,  0x4200, 0x00e2, 0x0003, TX_PWR_1P2_PA},   //-1dbm
    {-2,  0x4200, 0x00e3, 0x0004, TX_PWR_1P2_PA},   //-2dbm
    {-3,  0x4200, 0x00d2, 0x0004, TX_PWR_1P2_PA},   //-3dbm
    {-4,  0x4200, 0x00c3, 0x0005, TX_PWR_1P2_PA},   //-4dbm
    {-5,  0x4200, 0x00c4, 0x0006, TX_PWR_1P2_PA},   //-5dbm
    {-6,  0x4200, 0x00c2, 0x0006, TX_PWR_1P2_PA},   //-6dbm
    {-7,  0x4200, 0x0092, 0x0006, TX_PWR_1P2_PA},   //-7dbm
    {-8,  0x4200, 0x0072, 0x0006, TX_PWR_1P2_PA},   //-8dbm
    {-9,  0x4200, 0x0052, 0x0006, TX_PWR_1P2_PA},   //-9dbm
    {-10, 0x4200, 0x0032, 0x0006, TX_PWR_1P2_PA},   //-10dbm
    {-11, 0x4200, 0x0022, 0x0006, TX_PWR_1P2_PA},   //-11dbm
    {-12, 0x4200, 0x0012, 0x0006, TX_PWR_1P2_PA},   //-12dbm
};

static const struct RF_SYS_INIT_ITEM rf_sys_init_tbl[] =
{
};

const uint16_t rf_init_tbl_1[][3] =
{
    {0x0001, 0x0101, 0},    //timing_t1
    {0x0002, 0x0c0c, 0},
    {0x0003, 0x0e0d, 0},
    {0x0004, 0x131a, 0},
    {0x0005, 0x0008, 0},
    {0x0008, 0x1234, 0},    //timing_td
    {0x0009, 0x5556, 0},    //precharge time
    {0x000A, 0x0C01, 0},    //bt_timing_r1=3
    {0x000F, 0x0000, 0},    //pa pwd
    {0x0011, 0x4000, 0},    // lna mixer first en_s dr=0; bit2=0
    {0x0012, 0x0340, 0},    //cnt_time=001
    {0x0013, 0x0000, 0},    //capbank d
    {0x0018, 0X0060, 0},    //cnt_time_mode=00
    {0x0019, 0xFD40, 0},    //vco ictrl 010000
    {0x001B, 0x4000, 0},    //ictrl dr
    {0x0022, 0X5480, 0},    //fw_cal_start
    {0x0023, 0x01B0, 0},    // enable ble tx gain tbl
    {0x0024, 0x0000, 0},
    {0x0025, 0x8043, 0},
    {0x0028, 0x4002, 0},    // i2v cap dr=0; bit 15=0
    {0x0029, 0x8A10, 0},    //init1_aac
    {0x002B, 0x1000, 0},    //adc rstb
    {0x002c, 0x280E, 0},    // adc i2v gain dr=0; bit 0 9=0
    {0x0030, 0x0000, 0},    // lna_mixer first en dr=0; bit12=0
    {0x004D, 0x2c80, 0},    // lna hg en_s, lg rin_s, ldo res_s dr=0; bit 9 5 0 dr=0
    {0x004E, 0x1000, 0},    // adc gain_s,vco div2 dr=0; bit 4 9=0
    {0x004C, 0x2D6E, 0},    // mixer bias_s, rc_s, lna ic res_s, ic_t_s, hg_s dr=0; bit 0 4 9 14 15=0
    {0x004F, 0x3300, 0},    //adc ibias 2M/4M 0011
    {0x0051, 0x0200, 0},    // lna hg en dr=0; bit15=0
    {0x0052, 0xB730, 0},    // lna ic res, hg, ic_t dr=0;  bit0 7 11=0
    {0x0081, 0x8002, 0},    // i2v rin dr=0; bit 12=0
    {0x0082, 0x18AB, 0},    //increase LDO_VRES
    {0x008C, 0x0000, 0},    //reg_bt_dac_gain_ctrl
    {0x008D, 0x0020, 0},
    {0x008F, 0x080B, 0},
    {0X0090, 0X3E48, 0},    //bt_dac_ldo_vct
    {0x0091, 0x0A40, 0},    // adc_sel_op1_stb  adc rstb dac tmp0 dr=0; bit 14 15=0
    {0x0092, 0xF3A0, 0},    // mixer bias dr=0; bit 11=0
    {0x0095, 0x4838, 0},    //adc vcm 000
    {0x0099, 0x303B, 0},    //adc ibias 1M 0011
    {0x009A, 0X8088, 0},    //rfvco_vres_txbuf
    {0x009B, 0x5700, 0},    //kvco maxa
    {0x009C, 0x0071, 0},
    {0x009D, 0x0F04, 0},    // vco div2 dr=0 bit12 =0
    {0x009E, 0x3F78, 0},    //cap_ofst=1000
    {0x009F, 0xE911, 0},    //dac tst
    {0x00A0, 0x1908, 0},    //idn=1000
    {0x00A2, 0x18E8, 0},    //manu_ofsten=0,iup=1000
    {0x00A3, 0x0733, 0},
    {0x00A4, 0X8086, 0},    //vres_cp=0110
    {0x00A6, 0xC800, 0},
    {0x00BF, 0x0004, 0},
    {0x00C0, 0x03FC, 0},    //adc_sel_clk_4m/2m=1
    {0x00C1, 0x2D02, 0},    //adcclk
    {0x00C2, 0x5F50, 0},    //adc isel op1 2M
    {0x00C3, 0x0000, 0},
    {0x00CD, 0x0000, 0},
    {0x00CE, 0x41A2, 0},    //adc en_rst_sdm
    {0x00CF, 0x2000, 0},    // mixer rc dr=0; bit 9=0
    {0x00D3, 0x0121, 0},    //adc dwa 2m off
    {0x00D4, 0x0000, 0},    //adc dwa 4m off
    {0x00E5, 0xFF00, 0},    // adc dvdd 4M verG
    {0x0101, 0x0080, 0},    // adc_att_sel_dr=0; bit5=0
    {0x0110, 0x001F, 0},    //reg_pa_i_sel
    {0x0122, 0x0000, 0},    // lna lg rin dr=0; bit13=0
    {0x011D, 0x0000, 0},
    {0x0145, 0xC328, 0},    //adc inres
    {0x0147, 0x4065, 0},    //core_vres
    {0x015C, 0x0400, 0},
    {0x0160, 0x0000, 0},    // adc_sel_dac_ic_dr=0; bit 7=0
    {0x018C, 0x000F, 0},    //reg_bt_tmx_gain_ctrl
    {0x018D, 0x0000, 0},
    {0x01E6, 0x0410, 0},    //cnt time0
    {0x01E7, 0x0618, 0},    //cnt time1，enlarge high 3bit
    {0x01E8, 0x0410, 0},    //cnt time2
    {0x01E9, 0x0410, 0},    //cnt time3
    {0x01EA, 0x0410, 0},    //cnt time3
    {0x01EB, 0x0410, 0},    //cnt time4
    {0x03E1, 0x0180, 0},    //adc inres
    {0x03DE, 0xCC86, 0},    //ADC DAC IC
    {0x03DF, 0x3838, 0},    //ADC CORNER
    {0x03E2, 0x8686, 0},    //ADC STB
    {0x0408, 0x0820, 0},    //BT_RF_I2V_BYPASS
    {0x0409, 0x4400, 0},
    {0x040A, 0x0008, 0},    //dr 0 LNA precharge bias
    {0x040B, 0x0740, 0},    ////cal_thr=0001,1101
    {0x040D, 0x0764, 0},    //bt_i2v_sel_vcm_tx
    {0x0501, 0x0070, 0},    //mdll clkadc en 1M 2M 4M
    {0x050e, 0xC800, 0},    //mdll_clk_adc_sel_i_4m/2m=1
    {0x050d, 0x4000, 0},    //xtal_buf_rc_ssc_other=001000
    {0x0512, 0x0228, 0},    //xtal_buf_rc_ssc_CH70=001000
    {0x0519, 0x0208, 0},    //xtal_buf_rc_ssc_CH22/46=001000

#ifdef BT_RF_I2V_BYPASS
    //gain 0
    {0x006F, 0x08B2, 0},
    {0x0077, 0x7489, 0},
    {0x0134, 0x50A3, 0},
    {0x01A6, 0x00E3, 0},
    {0x01BE, 0x2988, 0},
    {0x01D6, 0x1969, 0},
    //gain1
    {0x0070, 0x0894, 0},
    {0x0078, 0x64F9, 0},
    {0x0135, 0x7893, 0},
    {0x01EC, 0x00E0, 0},
    {0x01BF, 0x2983, 0},
    {0x01D7, 0x1BFF, 0},
    //gain 2
    {0x0071, 0x0850, 0},
    {0x0079, 0x64F9, 0},
    {0x0136, 0x78A3, 0},
    {0x01A8, 0x00E0, 0},
    {0x01C0, 0x2983, 0},
    {0x01D8, 0x1BFF, 0},
    //gain 3
    {0x0072, 0x080C, 0},
    {0x007A, 0x64F8, 0},
    {0x0137, 0x7893, 0},
    {0x01A9, 0x00E0, 0},
    {0x01C1, 0x2983, 0},
    {0x01D9, 0x1BFF, 0},
    //gain 4
    {0x0073, 0x3809, 0},
    {0x007B, 0x64F8, 0},
    {0x0138, 0x7893, 0},
    {0x01AA, 0x00E0, 0},
    {0x01C2, 0x2983, 0},
    {0x01DA, 0x1BFF, 0},
    //gain 5
    {0x0074, 0xF809, 0},
    {0x007C, 0x64F8, 0},
    {0x0139, 0x7883, 0},
    {0x01AB, 0x00E4, 0},
    {0x01C3, 0x8688, 0},
    {0x01DB, 0x1BFF, 0},
    //gain6
    {0x0075, 0xF808, 0},
    {0x007D, 0x54F8, 0},
    {0x013A, 0x7853, 0},
    {0x01AC, 0x04E4, 0},
    {0x01C4, 0xE983, 0},
    {0x01DC, 0x1BFF, 0},
    //gain 7
    {0x0076, 0xF808, 0},
    {0x007E, 0x44F8, 0},
    {0x013B, 0x7833, 0},
    {0x01AD, 0x10E4, 0},
    {0x01C5, 0xE983, 0},
    {0x01DD, 0x1BFF, 0},
#else
    //gain 0
    {0x006f, 0x08B4, 0},
    {0x0077, 0x34F9, 0},
    {0x0134, 0x4093, 0},
    {0x01a6, 0x0063, 0},
    {0x01be, 0x8688, 0},
    {0x01d6, 0x1670, 0},
    //gain1
    {0x0070, 0x08B0, 0},
    {0x0078, 0x34F9, 0},
    {0x0135, 0x5093, 0},
    {0x01ec, 0x0063, 0},
    {0x01bf, 0x8688, 0},
    {0x01d7, 0x1670, 0},
    //gain 2
    {0x0071, 0x08B0, 0},
    {0x0079, 0x34F9, 0},
    {0x0136, 0x5093, 0},
    {0x01a8, 0x0060, 0},
    {0x01c0, 0x8688, 0},
    {0x01d8, 0x1670, 0},
    //gain 3
    {0x0072, 0x0850, 0},
    {0x007A, 0x34F9, 0},
    {0x0137, 0x50A3, 0},
    {0x01a9, 0x0060, 0},
    {0x01c1, 0x8688, 0},
    {0x01d9, 0x1670, 0},
    //gain 4
    {0x0073, 0x080C, 0},
    {0x007B, 0x34F8, 0},
    {0x0138, 0x5093, 0},
    {0x01aa, 0x0060, 0},
    {0x01c2, 0x8688, 0},
    {0x01da, 0x1670, 0},
    //gain 5
    {0x0074, 0x0809, 0},
    {0x007C, 0x34F8, 0},
    {0x0139, 0x5093, 0},
    {0x01ab, 0x0060, 0},
    {0x01c3, 0x8688, 0},
    {0x01db, 0x1670, 0},
    //gain6
    {0x0075, 0x5809, 0},
    {0x007D, 0x34F8, 0},
    {0x013a, 0x7883, 0},
    {0x01ac, 0x0064, 0},
    {0x01c4, 0x8688, 0},
    {0x01dc, 0x1670, 0},
    //gain 7
    {0x0076, 0x5809, 0},
    {0x007E, 0x34F8, 0},
    {0x013b, 0x7853, 0},
    {0x01ad, 0x0064, 0},
    {0x01c5, 0x8688, 0},
    {0x01dd, 0x1670, 0},
    // [i2v SW agc ini]
    {0x03e0, 0x0c38, 0}, //adc cap
    {0x0407, 0x0201, 0},
    {0x040E, 0x02f4, 0},

    {0x0097, 0x6FF0, 0},    //adc en recv 1M
    {0x00A1, 0x6D78, 0},    //adc en recv 2M/4M

    //BLE1M
    //rxgain0
    {0x0164, 0x0897, 0},    // rxgain0
    {0x016C, 0x0A4B, 0},    // rxgain1
    {0x0174, 0x4DC0, 0},    // rxgain2
    {0x017C, 0x3000, 0},    // rxgain3
    {0x01DE, 0x2904, 0},    // rxgain4
    {0x01C6, 0x0004, 0},    //rxgain4 nonbypass div2=4，dr=0

    //rxgain1
    {0x0165, 0x0873, 0},    //rxgain0
    {0x016D, 0x1EE9, 0},    //rxgain1
    {0x0175, 0x4DC0, 0},    // rxgain2
    {0x017D, 0x7000, 0},    // rxgain3
    {0x01DF, 0x0104, 0},    // rxgain4
    {0x01C7, 0x0004, 0},    //rxgain4 nonbypass div2=4，dr=0

    //rxgain2
    {0x0166, 0x2890, 0},    //rxgain0
    {0x016E, 0x1EE9, 0},    //rxgain1
    {0x0176, 0x4dc0, 0},    //rxgain2
    {0x017E, 0x7000, 0},    //rxgain3
    {0x01E0, 0x01FF, 0},    //rxgain4
    {0x01C8, 0x000F, 0},    //rxgain4 nonbypass div2=15，dr=0

    //rxgain3
    {0x0167, 0x2898, 0},    //rxgain0
    {0x016F, 0x1EE8, 0},    //rxgain1
    {0x0177, 0x4dc0, 0},    //rxgain2
    {0x017F, 0x7000, 0},    //rxgain3
    {0x01E1, 0x01FF, 0},    //rxgain4
    {0x01C9, 0x000F, 0},    //rxgain4 nonbypass div2=15，dr=0

    //rxgain4
    {0x0168, 0x2898, 0},    //rxgain0
    {0x0170, 0x1EE8, 0},    //rxgain1
    {0x0178, 0x49c0, 0},    //rxgain2
    {0x0180, 0x7003, 0},    //rxgain3
    {0x01E2, 0x01FF, 0},    //rxgain4
    {0x01CA, 0x000F, 0},    //rxgain4 nonbypass div2=15，dr=0

    //rxgain5
    {0x0169, 0x2898, 0},    //rxgain0
    {0x0171, 0x1EE8, 0},    //rxgain1
    {0x0179, 0x49c0, 0},    //rxgain2
    {0x0181, 0x700E, 0},    //rxgain3
    {0x01E3, 0x01FF, 0},    //rxgain4
    {0x01CB, 0x000F, 0},    //rxgain4 nonbypass div2=15，dr=0

    //rxgain6
    {0x016A, 0x2898, 0},    //rxgain0
    {0x0172, 0x0a48, 0},    //rxgain1
    {0x017A, 0x45c0, 0},    //rxgain2
    {0x0182, 0x707F, 0},    //rxgain3
    {0x01E4, 0x01FF, 0},    //rxgain4
    {0x01CC, 0x000F, 0},    //rxgain4 nonbypass div2=15，dr=0

    //rxgain7
    {0x016B, 0xF898, 0},    //rxgain0
    {0x0173, 0x0a48, 0},    //rxgain1
    {0x017B, 0xC1c0, 0},    //rxgain2
    {0x0183, 0x707F, 0},    //rxgain3
    {0x01E5, 0x01FF, 0},    //rxgain4
    {0x01CD, 0x000F, 0},    //rxgain4 nonbypass div2=15，dr=0


    //2M
    //rxgain tb0
    {0x0420, 0x0C38, 0},    //rxgain0
    {0x0421, 0x0180, 0},    //rxgain1
    {0x0422, 0x4738, 0},    //rxgain2
    {0x0423, 0x4738, 0},    //rxgain3
    {0x0424, 0x4738, 0},    //rxgain4
    {0x0425, 0x4738, 0},    //rxgain5
    {0x0426, 0x4738, 0},    //rxgain6
    {0x0427, 0xC738, 0},    //rxgain7

    //rxgain tb1
    {0x0430, 0x0C38, 0},    //rxgain0
    {0x0431, 0x0C38, 0},    //rxgain1
    {0x0432, 0x0C38, 0},    //rxgain2
    {0x0433, 0x0C38, 0},    //rxgain3
    {0x0434, 0x0C38, 0},    //rxgain4
    {0x0435, 0x0C38, 0},    //rxgain5
    {0x0436, 0x0C38, 0},    //rxgain6
    {0x0437, 0x0C38, 0},    //rxgain7

    //4M
    //rxgain tb0
    {0x0418, 0xC738, 0},    //rxgain0
    {0x0419, 0xC738, 0},    //rxgain1
    {0x041A, 0xC738, 0},    //rxgain2
    {0x041B, 0xC738, 0},    //rxgain3
    {0x041C, 0xC738, 0},    //rxgain4
    {0x041D, 0xC738, 0},    //rxgain5
    {0x041E, 0x0086, 0},    //rxgain6
    {0x041F, 0xC738, 0},    //rxgain7

    //rxgain tb1
    {0x0428, 0x0086, 0},    //rxgain0
    {0x0429, 0x0086, 0},    //rxgain1
    {0x042A, 0x0086, 0},    //rxgain2
    {0x042B, 0x0086, 0},    //rxgain3
    {0x042C, 0x0086, 0},    //rxgain4
    {0x042D, 0x0086, 0},    //rxgain5
    {0x042E, 0x0086, 0},    //rxgain6
    {0x042F, 0x0086, 0},    //rxgain7

    {0x0124, 0x0050, 0},    //i2v cap 2m gain0
    {0x0125, 0x0050, 0},    //i2v cap 2m gain1
    {0x0126, 0x0050, 0},    //i2v cap 2m gain2
    {0x0127, 0x0050, 0},    //i2v cap 2m gain3
    {0x0148, 0x0050, 0},    //i2v cap 2m gain4
    {0x0149, 0x0050, 0},    //i2v cap 2m gain5
    {0x014A, 0x0050, 0},    //i2v cap 2m gain6
    {0x014B, 0x0050, 0},    //i2v cap 2m gain7
#endif
};

#ifdef __HW_AGC__
const uint16_t rf_init_hw_agc_config[][3] =
{
    //tab1 sel
    {0x03DC, 0x1540, 0},

    {0x0049, 0x001F, 0}, //lna pdt gain,cmp_vref

#ifdef BT_RF_I2V_BYPASS
    {0x027D, 0x0001, 0},
    {0x0309, 0x0103, 0}, //s_hwagc lna apd en

    {0x030F, 0x0006, 0},
    {0x0310, 0x0258, 0},
    {0x0315, 0x7F7F, 0},
    {0x0316, 0x7F7F, 0},
    {0x0317, 0x4C01, 0},
    {0x037D, 0x0780, 0},
    {0x037E, 0x11D4, 0},
    {0x037F, 0x11D4, 0},
    {0x0380, 0x11D4, 0},
    {0x0381, 0x00C0, 0},
    {0x0387, 0x013B, 0},
    {0x0388, 0x00AD, 0},
    {0x0391, 0x1417, 0},
    {0x0390, 0x090C, 0},
    {0x03BF, 0X090C, 0},
    {0x0392, 0x7F0C, 0},
    {0x0393, 0x7F7F, 0},
    {0x0394, 0x0000, 0},
    {0x0395, 0x11D4, 0},
    {0x0396, 0x11D4, 0},
    {0x0397, 0x11D4, 0},
    {0x0398, 0x11D4, 0},
    {0x0399, 0x11D4, 0},
    {0x039A, 0x11D4, 0},
    {0x039B, 0x11D4, 0},
    {0x039C, 0x11D4, 0},
    {0x03DD, 0x0000, 0},

    //[HW AGC gain table]
    //gain 0
    //HW AGC gain table
    {0x031D, 0x08B7, 0},
    {0x031E, 0x1F09, 0},
    {0x031F, 0xCDC0, 0},
    {0x03A9, 0x5A00, 0},
    {0x03AA, 0x2988, 0},
    //gain 1
    {0x0320, 0x0890, 0},
    {0x0321, 0x1F09, 0},
    {0x0322, 0xD9C0, 0},
    {0x03AB, 0x7F80, 0},
    {0x03AC, 0x2988, 0},
    //gain 2
    {0x0323, 0x0850, 0},
    {0x0324, 0x1F09, 0},
    {0x0325, 0xD9C0, 0},
    {0x03AD, 0x7F80, 0},
    {0x03AE, 0x2988, 0},
    //gain 3
    {0x0326, 0x0808, 0},
    {0x0327, 0x1F08, 0},
    {0x0328, 0xD9C0, 0},
    {0x03AF, 0x7F80, 0},
    {0x03B0, 0x2988, 0},
    //gain 4
    {0x0329, 0x3808, 0},
    {0x032A, 0x1F08, 0},
    {0x032B, 0xD9C0, 0},
    {0x03B1, 0x7F80, 0},
    {0x03B2, 0x2988, 0},
    //gain 5
    {0x032C, 0xF808, 0},
    {0x032D, 0x1F08, 0},
    {0x032E, 0xD9C0, 0},
    {0x03B3, 0x7F80, 0},
    {0x03B4, 0x2988, 0},
    //gain6
    {0x032F,0xF808, 0},
    {0x0330,0x1F08, 0},
    {0x0331,0xD5C0, 0},
    {0x03B5,0x7F82, 0},
    {0x03B6,0xE988, 0},
    //gain 7
    {0x0332, 0xF808, 0},
    {0x0333, 0x1F08, 0},
    {0x0334, 0xD1C0, 0},
    {0x03B7, 0x7F88, 0},
    {0x03B8, 0xE988, 0},
    //jp offset
    {0x031A, 0x0009, 0},
    {0x028F, 0x0509, 0}, //jp offset BW 2M/4M

    //[HW AGC gain table1 for 4M]
    //gain 0
    {0x033D, 0x08B7, 0},
    {0x033E, 0x1F09, 0},
    {0x033F, 0xCDC0, 0},
    {0x03B9, 0x1800, 0},
    {0x03BA, 0x2988, 0},
    //gain 1
    {0x0340, 0x0890, 0},
    {0x0341, 0x1F09, 0},
    {0x0342, 0xD9C0, 0},
    {0x03BB, 0x1800, 0},
    {0x03BC, 0x2988, 0},
    //gain 2
    {0x0343, 0x0850, 0},
    {0x0344, 0x1F09, 0},
    {0x0345, 0xD9C0, 0},
    {0x03BD, 0x1800, 0},
    {0x03BE, 0x2988, 0},
    //gain 3
    {0x0346, 0x0808, 0},
    {0x0347, 0x1F08, 0},
    {0x0348, 0xD9C0, 0},
    {0x03BF, 0x1800, 0},
    {0x03C0, 0x2988, 0},
    //gain 4
    {0x0349, 0x3808, 0},
    {0x034A, 0x1F08, 0},
    {0x034B, 0xD9C0, 0},
    {0x03C1, 0x1800, 0},
    {0x03C2, 0x2988, 0},
    //gain 5
    {0x034C, 0xF808, 0},
    {0x034D, 0x1F08, 0},
    {0x034E, 0xD9C0, 0},
    {0x03C3, 0x1800, 0},
    {0x03C4, 0x2988, 0},
    //gain6
    {0x034F, 0xF808, 0},
    {0x0350, 0x1F08, 0},
    {0x0351, 0xD5C0, 0},
    {0x03C5, 0x1802, 0},
    {0x03C6, 0xE988, 0},
    //gain 7
    {0x0352, 0xF808, 0},
    {0x0353, 0x1F08, 0},
    {0x0354, 0xD1C0, 0},
    {0x03C7, 0x1808, 0},
    {0x03C8, 0xE988, 0},
    //jump point
    {0x0359, 0xB1AE, 0},
    {0x035A, 0xBAB7, 0},
    {0x035B, 0xC1BE, 0},
    {0x035C, 0x00C6, 0},
    //gain offset
    {0x0335, 0x2232, 0},
    {0x0336, 0x191E, 0},
    {0x0337, 0x1316, 0},
    {0x0338, 0x050D, 0},
    //jump point
    {0x0339, 0xB0AD, 0},
    {0x033A, 0xB8B5, 0},
    {0x033B, 0xC2BC, 0},
    {0x033C, 0x00CA, 0},
#else
    {0x0309, 0x0101, 0}, //s_hwagc lna apd en
    //gain offset
    {0x0335, 0x2433, 0},
    {0x0336, 0x1B21, 0},
    {0x0337, 0x1418, 0},
    {0x0338, 0x0D11, 0},

    //HW AGC gain table
    //gain 0
    {0x023D, 0x08B4, 0},
    {0x023E, 0x34F9, 0},
    {0x023F, 0x4093, 0},
    {0x02B9, 0x0063, 0},
    {0x02BA, 0x8688, 0},
    {0x02e6, 0x1670, 0},
    //gain 1
    {0x0240, 0x08B0, 0},
    {0x0241, 0x34F9, 0},
    {0x0242, 0x5093, 0},
    {0x02BB, 0x0063, 0},
    {0x02BC, 0x8688, 0},
    {0x02e7, 0x1670, 0},
    //gain 2
    {0x0243, 0x08B0, 0},
    {0x0244, 0x34F9, 0},
    {0x0245, 0x5093, 0},
    {0x02BD, 0x0060, 0},
    {0x02BE, 0x8688, 0},
    {0x02e8, 0x1670, 0},
    //gain 3
    {0x0246, 0x0850, 0},
    {0x0247, 0x34F9, 0},
    {0x0248, 0x50A3, 0},
    {0x02BF, 0x0060, 0},
    {0x02C0, 0x8688, 0},
    {0x02e9, 0x1670, 0},
    //gain 4
    {0x0249, 0x080C, 0},
    {0x024A, 0x34F8, 0},
    {0x024B, 0x5093, 0},
    {0x02C1, 0x0060, 0},
    {0x02C2, 0x8688, 0},
    {0x02ea, 0x1670, 0},
    //gain 5
    {0x024C, 0x0809, 0},
    {0x024D, 0x34F8, 0},
    {0x024E, 0x5093, 0},
    {0x02C3, 0x0060, 0},
    {0x02C4, 0x8688, 0},
    {0x02eb, 0x1670, 0},
    //gain6
    {0x024F, 0x5809, 0},
    {0x0250, 0x34F8, 0},
    {0x0251, 0x7883, 0},
    {0x02C5, 0x0064, 0},
    {0x02C6, 0x8688, 0},
    {0x02ec, 0x1670, 0},
    //gain 7
    {0x0252, 0x5809, 0},
    {0x0253, 0x34F8, 0},
    {0x0254, 0x7853, 0},
    {0x02C7, 0x0064, 0},
    {0x02C8, 0x8688, 0},
    {0x02ed, 0x1670, 0},
    //jump point edr2M
    {0x0259, 0xACA4, 0},
    {0x025A, 0xB4AF, 0},
    {0x025B, 0xBBB6, 0},
    {0x025C, 0x00C2, 0},
    //gain offset
    {0x0255, 0x2F34, 0},
    {0x0256, 0x2527, 0},
    {0x0257, 0x1D22, 0},
    {0x0258, 0x0E16, 0},
    //jp offset edr3M
    {0x021A, 0x0009, 0},

    //HW AGC gain table 4M i2v mode
    //HW AGC gain tab2
    //gain 0
    {0x025D, 0x08B7, 0},
    {0x025E, 0x1F09, 0},
    {0x025F, 0x10A3, 0},
    {0x02C9, 0x00C2, 0},
    {0x02CA, 0x0088, 0},
    //gain 1
    {0x0260, 0x0890, 0},
    {0x0261, 0x1F09, 0},
    {0x0262, 0x1093, 0},
    {0x02CB, 0x00C4, 0},
    {0x02CC, 0x0088, 0},
    //gain 2
    {0x0263, 0x0850, 0},
    {0x0264, 0x1F09, 0},
    {0x0265, 0x10A3, 0},
    {0x02CD, 0x00C0, 0},
    {0x02CE, 0x0088, 0},
    //gain 3
    {0x0266, 0x0808, 0},
    {0x0267, 0x1F08, 0},
    {0x0268, 0x1093, 0},
    {0x02CF, 0x00C4, 0},
    {0x02D0, 0x0088, 0},
    //gain 4
    {0x0269, 0x0808, 0},
    {0x026A, 0x1F08, 0},
    {0x026B, 0x1093, 0},
    {0x02D1, 0x00C1, 0},
    {0x02D2, 0x0088, 0},
    //gain 5
    {0x026C, 0x0808, 0},
    {0x026D, 0x1F08, 0},
    {0x026E, 0x1083, 0},
    {0x02D3, 0x00C8, 0},
    {0x02D4, 0x0088, 0},
    //gain6
    {0x026F, 0x0808, 0},
    {0x0270, 0x1F08, 0},
    {0x0271, 0x1053, 0},
    {0x02D5, 0x00C8, 0},
    {0x02D6, 0x0088, 0},
    //gain 7
    {0x0272, 0x0808, 0},
    {0x0273, 0x1F08, 0},
    {0x0274, 0x1033, 0},
    {0x02D7, 0x00C8, 0},
    {0x02D8, 0x0088, 0},
    //jump point edr2M
    {0x0279, 0xAFAC, 0},
    {0x027A, 0xB6B4, 0},
    {0x027B, 0xC0BB, 0},
    {0x027C, 0x00C6, 0},
    //gain offset
    {0x0275, 0x2934, 0},
    {0x0276, 0x2024, 0},
    {0x0277, 0x161B, 0},
    {0x0278, 0x090E, 0},
    //jp offset 4M
    {0x028f, 0x0510, 0}, //jp offset bw4M br1M
    {0x0290, 0x0410, 0}, //jp offset bw4M edr2M
    {0x0291, 0x0E19, 0}, //jp offset bw4M edr3M
    {0x0292, 0x7F10, 0}, //jp offset ble4M
    {0x02dc, 0x2a95, 0}, //2M 4M tab2

    //[1503 hwagc ctrl dig]
    {0x00d2, 0x1140, 0},       //i2v pdt vref 00
    {0x0218, 0x9555, 0},       //LNA pdt dr0
    {0x0209, 0x8152, 0},
    {0x020B, 0x0108, 0},
    {0x020F, 0x0006, 0},
    {0x0210, 0x0258, 0},
    {0x0211, 0x0108, 0},
    {0x0215, 0x0505, 0},
    {0x0216, 0x0505, 0},
    {0x0217, 0x4C01, 0},
    {0x027D, 0x0001, 0},
    {0x027E, 0x11D4, 0},
    {0x027F, 0x11D4, 0},
    {0x0280, 0x11D4, 0},
    {0x0281, 0x00C0, 0},
    {0x0283, 0x0084, 0},
    {0x0284, 0x0042, 0},
    {0x0287, 0x013B, 0},
    {0x0288, 0x00AD, 0},
    {0x028D, 0x0042, 0},
    {0x028E, 0x0084, 0},
    {0x0293, 0x7F7F, 0},
    {0x0294, 0x0000, 0},
    {0x0295, 0x11D4, 0},
    {0x0296, 0x11D4, 0},
    {0x0297, 0x11D4, 0},
    {0x0298, 0x11D4, 0},
    {0x0299, 0x11D4, 0},
    {0x029A, 0x11D4, 0},
    {0x029B, 0x11D4, 0},
    {0x029C, 0x11D4, 0},
    {0x02DD, 0x0001, 0},
#endif
};
#endif


#if (defined(__RF_APB_RESTORE_SUPPORT__) && defined(BT_LOG_POWEROFF))
void bt_drv_rf_inf_enable_auto_power_down(bool enable)
{
    uint16_t val = 0;

    btdrv_read_rf_reg(RF_REG_PU_RF_INF,&val);
    if(enable)
    {
        val &= ~(REG_PU_RF_INF_SW_DR|REG_ISO_EN_RF_INF_DR|REG_ISO_EN_TO_DIG_DR);
    }
    else
    {
        val |= REG_PU_RF_INF_SW_DR|REG_ISO_EN_RF_INF_DR|REG_ISO_EN_TO_DIG_DR;
    }

    btdrv_write_rf_reg(RF_REG_PU_RF_INF,val);
}
#endif //__RF_APB_RESTORE_SUPPORT__

void bt_drv_rf_set_bt_hw_agc_enable(bool enable)
{
#if 0
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0xb6,&val_e2);
    if(enable)
    {
        //open rf bt hw agc mode
        val_e2 |= (1<<1);
    }
    else
    {
        //close rf bt hw agc mode
        val_e2 &= ~(1<<1);
    }
    btdrv_write_rf_reg(0xb6,val_e2);
#endif
}

void bt_drv_rf_set_ble_hw_agc_enable(bool enable)
{
#if 0
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0xb7,&val_e2);
    if(enable)
    {
        //open rf ble hw agc mode
        val_e2 |= (1<<11);
    }
    else
    {
        //close rf ble hw agc mode
        val_e2 &= ~(1<<11);
    }
    btdrv_write_rf_reg(0xb7,val_e2);
#endif
}

void bt_drv_rf_set_afh_monitor_gain(void)
{
#if 0
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0x1e4,&val_e2);//reg_bt_i2v_rin_afh[6:4]
    //i2v_rin_afh = 2 i2v_stb_afh=1
    val_e2 &= ~(7<<4);
    val_e2 |= (2<<4);
    val_e2 &= ~(7<<12); //reg_bt_i2v_stb_afh[14:12]
    val_e2 |= (1<<12); //reg_bt_i2v_stb_afh[14:12]
    btdrv_write_rf_reg(0x1e4,val_e2);


    btdrv_read_rf_reg(0x3cb,&val_e2); //lna_ic = 0 , i2v_gain = 7  lna_ldo_ic = 0xb
    val_e2 &= ~(0xf<<0);//reg_bt_i2v_gain_afh[3:0]
    val_e2 |= (7<<0);

    val_e2 &= ~(3<<4);//reg_bt_adc_vin_sel_afh[5:4]
    val_e2 |= (1<<4);

    val_e2 &= ~(0xF<<8);//reg_bt_lna_ldo_ic_afh[11:8]
    val_e2 |= (0x5<<8);

    val_e2 &= ~(7<<12);// reg_bt_lna_ic_afh[14:12]

    val_e2 &= ~(3<<6);//reg_bt_adc_vin_sel_normal[7:6]
    val_e2 |= (1<<6);

    btdrv_write_rf_reg(0x3cb,val_e2);


    btdrv_read_rf_reg(0x3cc,&val_e2); //flt = 7, hg_en=0, hg=0, rpass=11, rin=00,rfflt=0

    val_e2 = 0x1E0C;

    btdrv_write_rf_reg(0x3cc,val_e2);
#endif
}

void btdrv_rf_init_xtal_fcap(uint32_t fcap)
{
    xtal_fcap = SET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL, fcap);
    btdrv_write_rf_reg(RF_REG_XTAL_FCAP, xtal_fcap);
    init_xtal_fcap = xtal_fcap;
}

uint32_t btdrv_rf_get_xtal_fcap(void)
{
    return GET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL);
}

struct btdrv_customer_rf_config_t btdrv_rf_customer_config;
void bt_drv_rf_sdk_init(void)
{
    BT_DRV_TRACE(0,"SDK default rf config");
    btdrv_rf_customer_config.config_xtal_en = false;//get value from NV
    btdrv_rf_customer_config.xtal_cap_val = 0;//get value from NV

    //config TX power in dbm
    btdrv_rf_customer_config.config_tx_pwr_en = true;
    btdrv_rf_customer_config.bt_tx_page_pwr = TX_PWR_8DBM;//in dbm

    btdrv_rf_customer_config.bt_tx_idx3_pwr = TX_PWR_12DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx2_pwr = TX_PWR_8DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx1_pwr = TX_PWR_4DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx0_pwr = TX_PWR_0DBM;//in dbm

    btdrv_rf_customer_config.le_tx_idx3_pwr = TX_PWR_12DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx2_pwr = TX_PWR_8DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx1_pwr = TX_PWR_4DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx0_pwr = TX_PWR_0DBM;//in dbm
    //init BT BLE convert table
    btdrv_txpwr_conv_tbl[0] = btdrv_rf_customer_config.bt_tx_idx0_pwr;
    btdrv_txpwr_conv_tbl[1] = btdrv_rf_customer_config.bt_tx_idx1_pwr;
    btdrv_txpwr_conv_tbl[2] = btdrv_rf_customer_config.bt_tx_idx2_pwr;
    btdrv_txpwr_conv_tbl[3] = btdrv_rf_customer_config.bt_tx_idx3_pwr;
    btdrv_txpwr_conv_tbl[4] = btdrv_rf_customer_config.bt_tx_idx3_pwr;
    btdrv_txpwr_conv_tbl[5] = btdrv_rf_customer_config.bt_tx_idx3_pwr;
    btdrv_txpwr_conv_tbl[6] = btdrv_rf_customer_config.bt_tx_idx3_pwr;
    btdrv_txpwr_conv_tbl[7] = btdrv_rf_customer_config.bt_tx_idx3_pwr;

    btdrv_ble_txpwr_conv_tbl[0] = btdrv_rf_customer_config.le_tx_idx0_pwr;
    btdrv_ble_txpwr_conv_tbl[1] = btdrv_rf_customer_config.le_tx_idx1_pwr;
    btdrv_ble_txpwr_conv_tbl[2] = btdrv_rf_customer_config.le_tx_idx2_pwr;
    btdrv_ble_txpwr_conv_tbl[3] = btdrv_rf_customer_config.le_tx_idx3_pwr;
    btdrv_ble_txpwr_conv_tbl[4] = btdrv_rf_customer_config.le_tx_idx3_pwr;
    btdrv_ble_txpwr_conv_tbl[5] = btdrv_rf_customer_config.le_tx_idx3_pwr;
    btdrv_ble_txpwr_conv_tbl[6] = btdrv_rf_customer_config.le_tx_idx3_pwr;
    btdrv_ble_txpwr_conv_tbl[7] = btdrv_rf_customer_config.le_tx_idx3_pwr;
}

void bt_drv_rf_set_customer_config(struct btdrv_customer_rf_config_t* config)
{
    memset(&btdrv_rf_customer_config, 0, sizeof(struct btdrv_customer_rf_config_t));
    if(config !=NULL)
    {
        btdrv_rf_customer_config = *config;
    }
    else
    {
        bt_drv_rf_sdk_init();
    }
}

static void bt_rf_txpwr_reg_get(uint8_t idx, uint16_t *txgain_tbl0, uint16_t *txgain_tbl1, uint16_t *txgain_tbl2)
{
    switch(idx)
    {
        case TX_PWR_IDX_0:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX0_REG;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX0_REG;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX0_REG;
            break;
        case TX_PWR_IDX_1:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX1_REG;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX1_REG;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX1_REG;
            break;
        case TX_PWR_IDX_2:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX2_REG;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX2_REG;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX2_REG;
            break;
        case TX_PWR_IDX_3:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX3_REG;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX3_REG;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX3_REG;
            break;
        case TX_PWR_IDX_4:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX4_REG;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX4_REG;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX4_REG;
            break;
        case TX_PWR_IDX_PAGE:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX6_REG;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX6_REG;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX6_REG;
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR tx pwr idx=%d", idx);
            break;
    }
}

static void ble_rf_txpwr_reg_get(uint8_t idx, uint16_t *txgain_tbl0, uint16_t *txgain_tbl1, uint16_t *txgain_tbl2)
{
    switch(idx)
    {
        case TX_PWR_IDX_0:
            *txgain_tbl0 = RF_LE_TX_PWR_TBL0_IDX0_REG;
            *txgain_tbl1 = RF_LE_TX_PWR_TBL1_IDX0_REG;
            *txgain_tbl2 = RF_LE_TX_PWR_TBL2_IDX0_REG;
            break;
        case TX_PWR_IDX_1:
            *txgain_tbl0 = RF_LE_TX_PWR_TBL0_IDX1_REG;
            *txgain_tbl1 = RF_LE_TX_PWR_TBL1_IDX1_REG;
            *txgain_tbl2 = RF_LE_TX_PWR_TBL2_IDX1_REG;
            break;
        case TX_PWR_IDX_2:
            *txgain_tbl0 = RF_LE_TX_PWR_TBL0_IDX2_REG;
            *txgain_tbl1 = RF_LE_TX_PWR_TBL1_IDX2_REG;
            *txgain_tbl2 = RF_LE_TX_PWR_TBL2_IDX2_REG;
            break;
        case TX_PWR_IDX_3:
            *txgain_tbl0 = RF_LE_TX_PWR_TBL0_IDX3_REG;
            *txgain_tbl1 = RF_LE_TX_PWR_TBL1_IDX3_REG;
            *txgain_tbl2 = RF_LE_TX_PWR_TBL2_IDX3_REG;
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR tx pwr idx=%d", idx);
            break;
    }
}

int rf_reg_txpwr_val_get(int8_t txpwr_dbm, uint16_t *txgain_tbl0, uint16_t *txgain_tbl1, uint16_t *txgain_tbl2, uint8_t *pa_sel)
{
    uint32_t nRet = -1;
    uint16_t i = 0;
    uint16_t tbl_size;
    tbl_size = sizeof(tx_pwr_select)/sizeof(tx_pwr_select[0]);
    for(i=0; i< tbl_size; i++){
        if(tx_pwr_select[i].pwr_dbm == txpwr_dbm){
            *txgain_tbl0 = tx_pwr_select[i].tx_gain_tbl0;
            *txgain_tbl1 = tx_pwr_select[i].tx_gain_tbl1;
            *txgain_tbl2 = tx_pwr_select[i].tx_gain_tbl2;
            *pa_sel = tx_pwr_select[i].pa_sel;
            nRet = 0;
            break;
        }
    }
    return nRet;
}

static void bt_drv_rf_bt_txpwr_set(uint8_t idx, int8_t val_in_dbm)
{
    uint8_t pa_sel = 0;
    uint16_t txgain_tbl0_val = 0;
    uint16_t txgain_tbl1_val = 0;
    uint16_t txgain_tbl2_val = 0;
    uint16_t txgain_tbl0_addr = 0;
    uint16_t txgain_tbl1_addr = 0;
    uint16_t txgain_tbl2_addr = 0;
    int nRet = 0;

    bt_rf_txpwr_reg_get(idx, &txgain_tbl0_addr, &txgain_tbl1_addr, &txgain_tbl2_addr);
    nRet = rf_reg_txpwr_val_get(val_in_dbm, &txgain_tbl0_val, &txgain_tbl1_val, &txgain_tbl2_val, &pa_sel);
    ASSERT(!nRet, "BT_DRV:pls write correct tx pwr");

    btdrv_write_rf_reg(txgain_tbl0_addr, txgain_tbl0_val);
    btdrv_write_rf_reg(txgain_tbl1_addr, txgain_tbl1_val);
    btdrv_write_rf_reg(txgain_tbl2_addr, txgain_tbl2_val);

    BT_DRV_TRACE(2, "BT:TX pwr in:%d dbm,[reg0=0x%x,out:0x%04x][reg1=0x%x,out:0x%04x][reg1=0x%x,out:0x%04x]",
    val_in_dbm, txgain_tbl0_addr, txgain_tbl0_val, txgain_tbl1_addr, txgain_tbl1_val, txgain_tbl2_addr, txgain_tbl2_val);
}

static void bt_drv_rf_ble_txpwr_set(uint8_t idx, int8_t val_in_dbm)
{
    uint8_t pa_sel = 0;
    uint16_t txgain_tbl0_val = 0;
    uint16_t txgain_tbl1_val = 0;
    uint16_t txgain_tbl2_val = 0;
    uint16_t txgain_tbl0_addr = 0;
    uint16_t txgain_tbl1_addr = 0;
    uint16_t txgain_tbl2_addr = 0;
    int nRet = 0;

    ble_rf_txpwr_reg_get(idx, &txgain_tbl0_addr, &txgain_tbl1_addr, &txgain_tbl2_addr);
    nRet = rf_reg_txpwr_val_get(val_in_dbm, &txgain_tbl0_val, &txgain_tbl1_val, &txgain_tbl2_val, &pa_sel);
    ASSERT(!nRet, "BT_DRV:pls write correct tx pwr");

    btdrv_write_rf_reg(txgain_tbl0_addr, txgain_tbl0_val);
    btdrv_write_rf_reg(txgain_tbl1_addr, txgain_tbl1_val);
    btdrv_write_rf_reg(txgain_tbl2_addr, txgain_tbl2_val);

    BT_DRV_TRACE(2, "BT:TX pwr in:%d dbm,[reg0=0x%x,out:0x%04x][reg1=0x%x,out:0x%04x][reg1=0x%x,out:0x%04x]",
    val_in_dbm, txgain_tbl0_addr, txgain_tbl0_val, txgain_tbl1_addr, txgain_tbl1_val, txgain_tbl2_addr, txgain_tbl2_val);
}

#ifdef REDUCE_EDGE_CHL_TXPWR
static void bt_drv_enable_edge_chl_txpwr_tbl(bool enable, uint16_t high_chl, uint16_t low_chl)
{
    if (enable) {
        BTRF_REG_SET_FIELD(0x40B, 1, 14, 1);             // enable edge chl txpwr tbl
        BTRF_REG_SET_FIELD(0x40C, 0x7F, 2, low_chl);     // low chl
        BTRF_REG_SET_FIELD(0x40C, 0x7F, 9, high_chl);    // high chl
    } else {
        BTRF_REG_SET_FIELD(0x40B, 1, 14, 0);     // disable edge chl txpwr tbl
    }
}

static void bt_rf_edge_chl_txpwr_reg_get(uint8_t idx, uint16_t *txgain_tbl0, uint16_t *txgain_tbl1, uint16_t *txgain_tbl2)
{
    switch(idx)
    {
        case TX_PWR_IDX_0:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX0_REG_EDGE_CHL;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX0_REG_EDGE_CHL;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX0_REG_EDGE_CHL;
            break;
        case TX_PWR_IDX_1:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX1_REG_EDGE_CHL;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX1_REG_EDGE_CHL;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX1_REG_EDGE_CHL;
            break;
        case TX_PWR_IDX_2:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX2_REG_EDGE_CHL;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX2_REG_EDGE_CHL;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX2_REG_EDGE_CHL;
            break;
        case TX_PWR_IDX_3:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX3_REG_EDGE_CHL;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX3_REG_EDGE_CHL;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX3_REG_EDGE_CHL;
            break;
        case TX_PWR_IDX_4:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX4_REG_EDGE_CHL;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX4_REG_EDGE_CHL;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX4_REG_EDGE_CHL;
            break;
        case TX_PWR_IDX_PAGE:
            *txgain_tbl0 = RF_BT_TX_PWR_TBL0_IDX6_REG_EDGE_CHL;
            *txgain_tbl1 = RF_BT_TX_PWR_TBL1_IDX6_REG_EDGE_CHL;
            *txgain_tbl2 = RF_BT_TX_PWR_TBL2_IDX6_REG_EDGE_CHL;
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR tx pwr idx=%d", idx);
            break;
    }
}

static void bt_drv_rf_bt_edge_chl_txpwr_set(uint8_t idx, int8_t val_in_dbm)
{
    uint8_t pa_sel = 0;
    uint16_t txgain_tbl0_val = 0;
    uint16_t txgain_tbl1_val = 0;
    uint16_t txgain_tbl2_val = 0;
    uint16_t txgain_tbl0_addr = 0;
    uint16_t txgain_tbl1_addr = 0;
    uint16_t txgain_tbl2_addr = 0;
    int nRet = 0;

    bt_rf_edge_chl_txpwr_reg_get(idx, &txgain_tbl0_addr, &txgain_tbl1_addr, &txgain_tbl2_addr);
    nRet = rf_reg_txpwr_val_get(val_in_dbm, &txgain_tbl0_val, &txgain_tbl1_val, &txgain_tbl2_val, &pa_sel);
    ASSERT(!nRet, "BT_DRV:pls write correct tx pwr");

    btdrv_write_rf_reg(txgain_tbl0_addr, txgain_tbl0_val);
    btdrv_write_rf_reg(txgain_tbl1_addr, txgain_tbl1_val);
    btdrv_write_rf_reg(txgain_tbl2_addr, txgain_tbl2_val);

    BT_DRV_TRACE(2, "BT:TX pwr in:%d dbm,[reg0=0x%x,out:0x%04x][reg1=0x%x,out:0x%04x]",
    val_in_dbm, txgain_tbl0_addr, txgain_tbl0_val, txgain_tbl1_addr, txgain_tbl1_val);
}
#endif

void bt_drv_tx_pwr_init(void)
{
    struct btdrv_customer_rf_config_t cfg = btdrv_rf_customer_config;

    if(cfg.config_tx_pwr_en == true)
    {
        //BT
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_0, cfg.bt_tx_idx0_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_1, cfg.bt_tx_idx1_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_2, cfg.bt_tx_idx2_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_3, cfg.bt_tx_idx3_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_PAGE, cfg.bt_tx_page_pwr);
        //BLE
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_0, cfg.le_tx_idx0_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_1, cfg.le_tx_idx1_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_2, cfg.le_tx_idx2_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_3, cfg.le_tx_idx3_pwr);

#ifdef REDUCE_EDGE_CHL_TXPWR
#define EDGE_CHL_REDUCE_PWR     2
#define EDGE_CHL_REDUCE_TXPWR_HIGH_CHL  78
#define EDGE_CHL_REDUCE_TXPWR_LOW_CHL  0

        bt_drv_enable_edge_chl_txpwr_tbl(true, EDGE_CHL_REDUCE_TXPWR_HIGH_CHL, EDGE_CHL_REDUCE_TXPWR_LOW_CHL);

        bt_drv_rf_bt_edge_chl_txpwr_set(TX_PWR_IDX_0, cfg.bt_tx_idx0_pwr - EDGE_CHL_REDUCE_PWR);
        bt_drv_rf_bt_edge_chl_txpwr_set(TX_PWR_IDX_1, cfg.bt_tx_idx1_pwr - EDGE_CHL_REDUCE_PWR);
        bt_drv_rf_bt_edge_chl_txpwr_set(TX_PWR_IDX_2, cfg.bt_tx_idx2_pwr - EDGE_CHL_REDUCE_PWR);
        bt_drv_rf_bt_edge_chl_txpwr_set(TX_PWR_IDX_3, cfg.bt_tx_idx3_pwr - EDGE_CHL_REDUCE_PWR);
        bt_drv_rf_bt_edge_chl_txpwr_set(TX_PWR_IDX_4, cfg.bt_tx_idx4_pwr - EDGE_CHL_REDUCE_PWR);
        bt_drv_rf_bt_edge_chl_txpwr_set(TX_PWR_IDX_PAGE, cfg.bt_tx_page_pwr - EDGE_CHL_REDUCE_PWR);
#endif
    }
    else
    {
        ASSERT(0, "BT_DRV:pls check TX pwr config");
    }
}

void bt_drv_rf_init_xtal_fcap(void)
{
    unsigned int xtal_fcap_temp = DEFAULT_XTAL_FCAP;
    uint16_t xtal_val = 0;

    if(btdrv_rf_customer_config.config_xtal_en == true)
    {
        xtal_fcap_temp = btdrv_rf_customer_config.xtal_cap_val;
        BT_DRV_TRACE(1,"btdrv customer set xtal fcap=0x%x", xtal_fcap_temp);
    }
    else
    {
#ifdef __RF_INIT_XTAL_CAP_FROM_NV__
        if (!nvrec_dev_get_xtal_fcap(&xtal_fcap_temp))
        {
            BT_DRV_TRACE(2,"%s xtal cap=0x%x", __func__, xtal_fcap_temp);
        }
        else
#endif
        {
            xtal_fcap_temp = DEFAULT_XTAL_FCAP;
            BT_DRV_TRACE(1,"%s failed", __func__);
        }
    }

    for (uint32_t i = 0; i<9; i++){
        if ((xtal_fcap_temp) >> (i) & 1) {
            xtal_val |= 1 << i;
            btdrv_rf_init_xtal_fcap(xtal_val);
        }
        btdrv_delay(1);
    }
}

void bt_drv_ble_adv_txpwr_via_advhdl(uint8_t adv_hdl, uint8_t idx, int8_t txpwr_dbm)
{

    BT_DRV_TRACE(2,"set adv_txpwr_via_advhdl hdl 0x%02x idx %d dbm %d", adv_hdl, idx, txpwr_dbm);

    if(idx > 5)
    {
        return;
    }

    if(txpwr_dbm <-3)
    {
        txpwr_dbm += 18;
        bt_drv_reg_op_low_txpwr(true, FACTOR_ATTENUATION_18DBM, BLE_ADV_LOW_TXPWR_MODE, adv_hdl);
    }
    else
    {
        bt_drv_reg_op_low_txpwr(false, FACTOR_ATTENUATION_18DBM, BLE_ADV_LOW_TXPWR_MODE, adv_hdl);
    }

    bt_drv_rf_ble_txpwr_set(idx, txpwr_dbm);
}

void btdrv_rf_register_init(const uint16_t rf_init_tbl_p[][3], uint16_t tbl_size)
{
    uint16_t i;
    uint16_t value = 0;

    for (i=0; i< tbl_size; i++) {
        btdrv_write_rf_reg(rf_init_tbl_p[i][0],rf_init_tbl_p[i][1]);
        if(rf_init_tbl_p[i][2] !=0)
            btdrv_delay(rf_init_tbl_p[i][2]);//delay
        btdrv_read_rf_reg(rf_init_tbl_p[i][0],&value);
        BT_DRV_TRACE(2,"reg=%x,v=%x",rf_init_tbl_p[i][0],value);
    }
}

uint8_t btdrv_rf_init(void)
{
    uint16_t value = 0;
    const uint16_t (*rf_init_tbl_p)[3] = NULL;
    uint32_t tbl_size = 0;
    uint8_t i;

    //system RF register init
    for (i = 0; i < ARRAY_SIZE(rf_sys_init_tbl); i++){
        btdrv_read_rf_reg(rf_sys_init_tbl[i].reg, &value);
        value = (value & ~rf_sys_init_tbl[i].mask) | (rf_sys_init_tbl[i].set & rf_sys_init_tbl[i].mask);
        if (rf_sys_init_tbl[i].delay){
            btdrv_delay(rf_sys_init_tbl[i].delay);
        }
        btdrv_write_rf_reg(rf_sys_init_tbl[i].reg, value);
    }
    //common RF register init
    rf_init_tbl_p = &rf_init_tbl_1[0];
    tbl_size = sizeof(rf_init_tbl_1)/sizeof(rf_init_tbl_1[0]);
    btdrv_rf_register_init(rf_init_tbl_p, tbl_size);

#ifdef __HW_AGC__
    rf_init_tbl_p = &rf_init_hw_agc_config[0];
    tbl_size = sizeof(rf_init_hw_agc_config)/sizeof(rf_init_hw_agc_config[0]);
    btdrv_rf_register_init(rf_init_tbl_p, tbl_size);
#endif
#ifdef __NEW_SWAGC_MODE__
    //open rf BT sync agc mode
    bt_drv_rf_set_bt_sync_agc_enable(true);
#endif
    //customer option
    bt_drv_rf_set_customer_config(NULL);
    //TX power init
    bt_drv_tx_pwr_init();
    //XTAL cap init
    bt_drv_rf_init_xtal_fcap();

    return 0;
}

#ifdef __NEW_SWAGC_MODE__
void bt_drv_rf_set_bt_sync_agc_enable(bool enable)
{
    uint16_t val = 0;
    btdrv_read_rf_reg(RF_REG_SYNC_AGC, &val);
    if(enable)
    {
        //open rf new sync agc mode
        val |= (1<<3);
    }
    else
    {
        //close rf new sync agc mode
        val &= ~(1<<3);
    }
    btdrv_write_rf_reg(RF_REG_SYNC_AGC, val);
}
#endif

struct TX_PWR_CALIB_ITEM
{
    uint32_t txpwr_dig_reg;
    uint16_t mask;
    uint16_t shift;
};

static const struct TX_PWR_CALIB_ITEM tx_pwr_calib[] =
{
        {BESMDM_INT_DIG_GAIN0_ADDR, 0x3FF, 0},
        {BESMDM_INT_DIG_GAIN0_ADDR, 0x3FF, 10},
        {BESMDM_INT_DIG_GAIN0_ADDR, 0x3FF, 20},
        {BESMDM_INT_DIG_GAIN1_ADDR, 0x3FF, 0},
        {BESMDM_INT_DIG_GAIN1_ADDR, 0x3FF, 10},
        {BESMDM_INT_DIG_GAIN1_ADDR, 0x3FF, 20},
        {BESMDM_INT_DIG_GAIN2_ADDR, 0x3FF, 0},
        {BESMDM_INT_DIG_GAIN2_ADDR, 0x3FF, 10},
};

static void btdrv_tx_power_rf_comp(const uint16_t rf_tx_gain)
{
    uint16_t read_val = 0;
    const uint16_t bt_gain_addr_base = RF_BT_TX_PWR_TBL1_IDX0_REG;
    const uint16_t ble_gain_addr_base = RF_LE_TX_PWR_TBL1_IDX0_REG;
    uint16_t tx_gain_val = 0;

    for (uint8_t i = 0; i <= BT_MAX_TX_PWR_IDX; i++) {
        //bt
        btdrv_read_rf_reg((bt_gain_addr_base + i), &read_val);
        tx_gain_val = (read_val >> 4) & 0x1F;
        read_val = (uint16_t)((float)rf_tx_gain * (float)tx_gain_val / 21.0);   // The power calibration is based on 14dbm, and the other idx are linearly mapped
        BTRF_REG_SET_FIELD((bt_gain_addr_base + i), 0x1F, 4, read_val);
    }
    for (uint8_t i = 0; i <= BLE_MAX_TX_PWR_IDX; i++) {
        //ble
        btdrv_read_rf_reg((ble_gain_addr_base + i), &read_val);
        tx_gain_val = (read_val >> 4) & 0x1F;
        read_val = (uint16_t)((float)rf_tx_gain * (float)tx_gain_val / 21.0);
        BTRF_REG_SET_FIELD((ble_gain_addr_base + i), 0x1F, 4, read_val);
    }
}

void btdrv_txpower_calib(void)
{
    uint16_t read_val;
    int16_t calib_val;
    uint16_t dig_reg_calib_val = 0;
    int16_t bt_pwr_freql, bt_pwr_freqm, bt_pwr_freqh;
    float bt_pwr_avg = 0.0f;

    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &read_val);
    bt_pwr_freql = read_val & 0x7;
    bt_pwr_freqm = (read_val >> 4) & 0x7;
    bt_pwr_freqh = (read_val >> 8) & 0x7;

    bt_pwr_freql = getbit(read_val, 3)  ? bt_pwr_freql : (-bt_pwr_freql);
    bt_pwr_freqm = getbit(read_val, 7)  ? bt_pwr_freqm : (-bt_pwr_freqm);
    bt_pwr_freqh = getbit(read_val, 11) ? bt_pwr_freqh : (-bt_pwr_freqh);

    bt_pwr_avg = ((float)(bt_pwr_freql + bt_pwr_freqm + bt_pwr_freqh) / 3.0f);

    if (bt_pwr_avg > 0) {
        calib_val = (int16_t) (bt_pwr_avg + 0.5f);
    } else {
        calib_val = (int16_t) (bt_pwr_avg - 0.5f);
    }

    TRACE(3, "%s efuse 5 val=0x%x, calib_val=%d",__func__, read_val, calib_val);

    if (calib_val > 4) {
        calib_val = 4;
    } else if (calib_val < -4) {
        calib_val = -4;
    }

    // High transmission power reduces digital gain
    if (calib_val > 0 && calib_val <= 4) {
        if (calib_val == 1) {
            dig_reg_calib_val = 0x1E4;
        } else if (calib_val == 2) {
            dig_reg_calib_val = 0x1CA;
        } else if (calib_val == 3) {
            dig_reg_calib_val = 0x1B2;
        } else if (calib_val == 4) {
            dig_reg_calib_val = 0x19B;
        }

        for (int i = 0; i < ARRAY_SIZE(tx_pwr_calib); i++) {
            BTDIGITAL_REG_SET_FIELD(tx_pwr_calib[i].txpwr_dig_reg, tx_pwr_calib[i].mask, tx_pwr_calib[i].shift, dig_reg_calib_val);
        }
    } else if (calib_val == -1) {
        btdrv_tx_power_rf_comp(0x19);
    } else if (calib_val == -2) {
        btdrv_tx_power_rf_comp(0x1F);
    } else if (calib_val == -3) {
        btdrv_tx_power_rf_comp(0x1F);
    } else if (calib_val == -4) {
        btdrv_tx_power_rf_comp(0x1F);
    }
}

