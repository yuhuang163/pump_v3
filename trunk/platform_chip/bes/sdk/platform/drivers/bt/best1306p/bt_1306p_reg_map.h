/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __BT_1306P_REG_MAP_H__
#define __BT_1306P_REG_MAP_H__

#ifndef ASSERT_ERR
#define ASSERT_ERR(cond)                             { if (!(cond)) { BT_DRV_TRACE(2,"line is %d file is %s", __LINE__, __FILE__); } }
#endif

/***************************************************************************
 *BES 1306P CHIP base address define
 ****************************************************************************/
#define  CFG_MODEM_BASE_ADDR  0xd0350000

#define  CFG_GPIO_BASE_ADDR  0xd0340000

#define  CFG_AUDMA_BASE_ADDR  0xd0100000

#define  CFG_UART0_BASE_ADDR  0xd0300000

#define  CFG_CMU_BASE_ADDR  0xd0330000

#define  CFG_WDT_BASE_ADDR  0xd0320000

#define  CFG_APB_RFINF_BASE_ADDR  0xd0390000

#define  CFG_AON_BASE_ADDR  0xd0360000
//1306P no transq
//#define  CFG_TRANSQ_BASE_ADDR  0xd0380000

#define  CFG_IQ_BASE_ADDR  0xd0370000

#define  CFG_CPU_DUMP_BASE_ADDR  0xd0700000

#define  EM_BASE_ADDR 0xd0200000

#define  RAM_BASE_ADDR  0xc0000000

#define  ROM_BASE_ADDR  0xa0000000

#define  BT_CORE_BASE_ADDR 0xd0220000

/***************************************************************************
 * BT CMU registers
 ****************************************************************************/

#define CMU_CLKREG_ADDR   (CFG_CMU_BASE_ADDR + 0x38)
__STATIC_FORCEINLINE void cmu_pol_clk_adc_setf(uint8_t polclkadc)
{
    ASSERT_ERR((((uint32_t)polclkadc << 12) & ~((uint32_t)0x00001000)) == 0);
    REG_PL_WR(CMU_CLKREG_ADDR, (REG_PL_RD(CMU_CLKREG_ADDR) & ~((uint32_t)0x00001000)) | ((uint32_t)polclkadc << 12));
}

/**
 * @brief CMU_IRQ_CNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     04    BTCORE_WAKEUP_REQ   0
 *     03     BT2MCU_DATA_IND1   0
 *     02     BT2MCU_DATA_IND0   0
 *     01    MCU2BT_DATA_DONE1   0
 *     00    MCU2BT_DATA_DONE0   0
 * </pre>
 */
#define CMU_CMU_IRQ_CNTL_ADDR  (CFG_CMU_BASE_ADDR + 0x50)

#define CMU_REG_54_CNTL_ADDR  (CFG_CMU_BASE_ADDR + 0x54)

/**
 * @brief IRQ_STATE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     27   sys2bt_data1_intr_msk   0
 *     26   sys2bt_data_intr_msk   0
 *     25   bt2sys_data1_intr_msk   0
 *     24   bt2sys_data_intr_msk   0
 *     23    sys2bt_data1_intr   0
 *     22     sys2bt_data_intr   0
 *     21    bt2sys_data1_intr   0
 *     20     bt2sys_data_intr   0
 *     19   sys2bt_data1_msk_set   0
 *     18   sys2bt_data_msk_set   0
 *     17   bt2sys_data1_msk_set   0
 *     16   bt2sys_data_msk_set   0
 *     11   mcu2bt_data1_intr_msk   0
 *     10   mcu2bt_data_intr_msk   0
 *     09   bt2mcu_data1_intr_msk   0
 *     08   bt2mcu_data_intr_msk   0
 *     07    mcu2bt_data1_intr   0
 *     06     mcu2bt_data_intr   0
 *     05    bt2mcu_data1_intr   0
 *     04     bt2mcu_data_intr   0
 *     03   mcu2bt_data1_msk_set   0
 *     02   mcu2bt_data_msk_set   0
 *     01   bt2mcu_data1_msk_set   0
 *     00   bt2mcu_data_msk_set   0
 * </pre>
 */
#define BT_CMU_IRQ_STATE_ADDR   (CFG_CMU_BASE_ADDR + 0xa0)

__STATIC_FORCEINLINE void bt_cmu_sys_2bt_data_1_msk_setf(uint8_t sys2btdata1mskset)
{
    REG_PL_WR(BT_CMU_IRQ_STATE_ADDR, (REG_PL_RD(BT_CMU_IRQ_STATE_ADDR) & ~((uint32_t)0x00080000)) | ((uint32_t)sys2btdata1mskset << 19));
}

__STATIC_FORCEINLINE void bt_cmu_mcu_2bt_data_1_msk_setf(uint8_t mcu2btdata1mskset)
{
    REG_PL_WR(BT_CMU_IRQ_STATE_ADDR, (REG_PL_RD(BT_CMU_IRQ_STATE_ADDR) & ~((uint32_t)0x00000008)) | ((uint32_t)mcu2btdata1mskset << 3));
}

#define BT_CONTROLLER_CRASH_DUMP_ADDR_PC  (CFG_CMU_BASE_ADDR + 0xbc)
/***************************************************************************
 * BT modem registers
 ****************************************************************************/
#define MODEM_BLE_TX_POWR_ADDR (CFG_MODEM_BASE_ADDR + 0xC4)

/***************************************************************************
 *BES ARM cortex M33 BT ROM Patch registers
 ****************************************************************************/

#define SRAM_REMAPPED_ADDR                  (0xa0200000)

#define BTDRV_PATCH_EN_REG                  0xa01ffe00

//instruction patch compare src address
#define BTDRV_PATCH_INS_COMP_ADDR_START     0xa01ffe00

#define BTDRV_PATCH_INS_REMAP_ADDR_START    0xa01fff00

/***************************************************************************
 *BT SRAM define
 ****************************************************************************/

#define BT_SRAM_SIZE 0x10000

#ifndef BT_CONTROLLER_CRASH_DUMP_ADDR_BASE
#define BT_CONTROLLER_CRASH_DUMP_ADDR_BASE  (RAM_BASE_ADDR + 0x4060)
#endif
/***************************************************************************
 * BT EM define
 ****************************************************************************/
//1306P/1307 ISO CON 2
#define EM_SIZE 0xa000

/***************************************************************************
 * rwip error registers
 ****************************************************************************/
#define BT_ERRORTYPESTAT_ADDR          (BT_CORE_BASE_ADDR + 0x460)
#define BLE_ERRORTYPESTAT_ADDR         (BT_CORE_BASE_ADDR + 0x860)
/***************************************************************************
 * BT CORE registers
 ****************************************************************************/
/**
 * @brief DIAGCNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31             DIAG3_EN   0
 *  29:24                DIAG3   0x0
 *     23             DIAG2_EN   0
 *  21:16                DIAG2   0x0
 *     15             DIAG1_EN   0
 *  13:08                DIAG1   0x0
 *     07             DIAG0_EN   0
 *  05:00                DIAG0   0x0
 * </pre>
 */
#define IP_DIAGCNTL_ADDR               (BT_CORE_BASE_ADDR + 0x50)

#define IP_SWPROFILING_ADDR            (BT_CORE_BASE_ADDR + 0x64)

#define BT_REG_CLKCAP_REG_ADDR         (BT_CORE_BASE_ADDR + 0xE00)
#define BT_BES_CLK_REG1_ADDR           (BT_CORE_BASE_ADDR + 0xE04)
#define BT_BES_CLK_REG2_ADDR           (BT_CORE_BASE_ADDR + 0xE08)
#define BT_REG_CLKNCNT_CAP4_REG_ADDR   (BT_CORE_BASE_ADDR + 0xE34)
#define BT_BES_FINECNT_CAP4_REG_ADDR   (BT_CORE_BASE_ADDR + 0xE38)
#define BT_REG_CLKNCNT_CAP5_REG_ADDR   (BT_CORE_BASE_ADDR + 0xE3C)
#define BT_BES_FINECNT_CAP5_REG_ADDR   (BT_CORE_BASE_ADDR + 0xE40)
#define BT_REG_CLKNCNT_CAP6_REG_ADDR   (BT_CORE_BASE_ADDR + 0xE44)
#define BT_BES_FINECNT_CAP6_REG_ADDR   (BT_CORE_BASE_ADDR + 0xE48)
#define BT_REG_CLKNCNT_CAP7_REG_ADDR   (BT_CORE_BASE_ADDR + 0xE4C)
#define BT_BES_FINECNT_CAP7_REG_ADDR   (BT_CORE_BASE_ADDR + 0xE50)


/**
 * @brief BES_TESTMODE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00          bt_testmode   0x0
 * </pre>
 */
#define BT_BES_TESTMODE_ADDR   (BT_CORE_BASE_ADDR + 0xC00)

/**
 * @brief BES_CNTL0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31    reg_ecc_dsb_sniff   0
 *     30       reg_ecc_dir_dr   0
 *     29          reg_ecc_dir   0
 *     28    reg_hwspi_mask_fa   0
 *     27   reg_setreg_rxneg_en   0
 *     26   reg_setreg_txneg_en   0
 *     25   reg_setreg_rxpos_en   0
 *     24   reg_setreg_txpos_en   0
 *     23      reg_wide_afh_en   0
 *     22   reg_rxgain_index_dr   0
 *     21   reg_txpwr_index_dr   0
 *     20      reg_txarqn_mask   0
 *     19     reg_dp_right_sel   0
 *     18   reg_testloop_patch_en   0
 *     17          reg_bw2m_dr   0
 *     16             reg_bw2m   0
 *  15:12     reg_rxgain_index   0x0
 *  11:08      reg_txpwr_index   0x0
 *  05:00   bt_testmode_ramptime   0x0
 * </pre>
 */
#define BT_BES_CNTL0_ADDR   (BT_CORE_BASE_ADDR + 0xC04)

__STATIC_FORCEINLINE void bt_bes_cntl0_reg_rxgain_index_dr_setf(uint8_t regrxgainindexdr)
{
    ASSERT_ERR((((uint32_t)regrxgainindexdr << 22) & ~((uint32_t)0x00400000)) == 0);
    REG_BT_WR(BT_BES_CNTL0_ADDR, (REG_BT_RD(BT_BES_CNTL0_ADDR) & ~((uint32_t)0x00400000)) | ((uint32_t)regrxgainindexdr << 22));
}

__STATIC_FORCEINLINE void bt_bes_cntl0_reg_txpwr_index_dr_setf(uint8_t regtxpwrindexdr)
{
    ASSERT_ERR((((uint32_t)regtxpwrindexdr << 21) & ~((uint32_t)0x00200000)) == 0);
    REG_BT_WR(BT_BES_CNTL0_ADDR, (REG_BT_RD(BT_BES_CNTL0_ADDR) & ~((uint32_t)0x00200000)) | ((uint32_t)regtxpwrindexdr << 21));
}

#define BT_BES_CNTL1_ADDR              (BT_CORE_BASE_ADDR + 0xC08)
/**
 * @brief BES_CNTL2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24      reg_fatxpwrupct   0x0
 *  23:16      reg_farxpwrupct   0x0
 *  15:11       reg_cnt_pkt_us   0x0
 *  10:07      reg_txon_off_fa   0x0
 *     06      reg_acl_fa_only   0
 *     05     reg_esco_fa_only   0
 *     04            reg_fa_en   0
 *     03       reg_tx_silence   0
 *     02    reg_tx_silence_dr   0
 *  01:00   reg_tx_silence_sel   0x0
 * </pre>
 */
#define BT_BES_CNTL2_ADDR              (BT_CORE_BASE_ADDR + 0xC0C)

__STATIC_FORCEINLINE void bt_bes_cntl2_reg_fatxpwrupct_setf(uint8_t regfatxpwrupct)
{
    ASSERT_ERR((((uint32_t)regfatxpwrupct << 24) & ~((uint32_t)0xFF000000)) == 0);
    REG_BT_WR(BT_BES_CNTL2_ADDR, (REG_BT_RD(BT_BES_CNTL2_ADDR) & ~((uint32_t)0xFF000000)) | ((uint32_t)regfatxpwrupct << 24));
}

__STATIC_FORCEINLINE void bt_bes_cntl2_reg_farxpwrupct_setf(uint8_t regfarxpwrupct)
{
    ASSERT_ERR((((uint32_t)regfarxpwrupct << 16) & ~((uint32_t)0x00FF0000)) == 0);
    REG_BT_WR(BT_BES_CNTL2_ADDR, (REG_BT_RD(BT_BES_CNTL2_ADDR) & ~((uint32_t)0x00FF0000)) | ((uint32_t)regfarxpwrupct << 16));
}

__STATIC_FORCEINLINE void bt_bes_cntl2_reg_cnt_pkt_us_setf(uint8_t regcntpktus)
{
    ASSERT_ERR((((uint32_t)regcntpktus << 11) & ~((uint32_t)0x0000F800)) == 0);
    REG_BT_WR(BT_BES_CNTL2_ADDR, (REG_BT_RD(BT_BES_CNTL2_ADDR) & ~((uint32_t)0x0000F800)) | ((uint32_t)regcntpktus << 11));
}

/**
 * @brief BES_CNTL3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31      poll_null_fa_en   0
 *     30       reg_fa_gain_en   0
 *     29      reg_fa_txpwr_en   0
 *  28:25          reg_fatxpwr   0x0
 *  24:21         reg_farxgain   0x0
 *  20:16     reg_fec_comp_bit   0x0
 *  15:08     reg_cnt_ecc_act2   0x0
 *  07:00     reg_cnt_ecc_act1   0x0
 * </pre>
 */

#define BT_BES_CNTL3_ADDR              (BT_CORE_BASE_ADDR + 0xC10)

__STATIC_FORCEINLINE void bt_bes_cntl3_reg_farxgain_setf(uint8_t regfarxgain)
{
    ASSERT_ERR((((uint32_t)regfarxgain << 21) & ~((uint32_t)0x01E00000)) == 0);
    REG_BT_WR(BT_BES_CNTL3_ADDR, (REG_BT_RD(BT_BES_CNTL3_ADDR) & ~((uint32_t)0x01E00000)) | ((uint32_t)regfarxgain << 21));
}

__STATIC_FORCEINLINE void bt_bes_cntl3_reg_fatxpwr_setf(uint8_t regfatxpwr)
{
    ASSERT_ERR((((uint32_t)regfatxpwr << 25) & ~((uint32_t)0x1E000000)) == 0);
    REG_BT_WR(BT_BES_CNTL3_ADDR, (REG_BT_RD(BT_BES_CNTL3_ADDR) & ~((uint32_t)0x1E000000)) | ((uint32_t)regfatxpwr << 25));
}

__STATIC_FORCEINLINE void bt_bes_cntl3_reg_fa_txpwr_en_setf(uint8_t regfatxpwren)
{
    ASSERT_ERR((((uint32_t)regfatxpwren << 29) & ~((uint32_t)0x20000000)) == 0);
    REG_BT_WR(BT_BES_CNTL3_ADDR, (REG_BT_RD(BT_BES_CNTL3_ADDR) & ~((uint32_t)0x20000000)) | ((uint32_t)regfatxpwren << 29));
}

__STATIC_FORCEINLINE void bt_bes_cntl3_reg_fa_gain_en_setf(uint8_t regfagainen)
{
    ASSERT_ERR((((uint32_t)regfagainen << 30) & ~((uint32_t)0x40000000)) == 0);
    REG_BT_WR(BT_BES_CNTL3_ADDR, (REG_BT_RD(BT_BES_CNTL3_ADDR) & ~((uint32_t)0x40000000)) | ((uint32_t)regfagainen << 30));
}

/**
 * @brief BES_CNTL5 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31   fatx_pull_pre_on_en   0
 *     30            ud_bid_en   0
 *     29     hec_winsz_fix_en   0
 *     28   null_ack_stop_sco_retx   0
 *     27   fa_done_clr_xsco_en   0
 *     26          fastlock_en   0
 *     25        multifa_mode1   0
 *     24        multifa_mode0   0
 *  23:22       mode1_fa_times   0x0
 *  21:17   more_fa_start_mode1   0x0
 *     16           fa_mp_mode   0
 *     15       normal_mp_mode   0
 *     14         ble_hwspi_en   0
 *     13          bt_hwspi_en   0
 *     12      pcm_read_tog_en   0
 *     11     pcm_data_flag_en   0
 *     10      sbc_eccdsb_mask   0
 *     09         sbc_sniff_en   0
 *     08               sbc_en   0
 *     07    reg_fa_timeout_en   0
 *  06:03   reg_fa_timeout_num   0x0
 *     02     reg_set_btclk_en   0
 *     01   reg_acl_pkt_ecc_en   0
 *     00         reg_ecc_mode   0
 * </pre>
 */
#define BT_BES_CNTL5_ADDR              (BT_CORE_BASE_ADDR + 0xC18)

__STATIC_FORCEINLINE void bt_bes_cntl5_fa_mp_mode_setf(uint8_t fampmode)
{
    ASSERT_ERR((((uint32_t)fampmode << 16) & ~((uint32_t)0x00010000)) == 0);
    REG_BT_WR(BT_BES_CNTL5_ADDR, (REG_BT_RD(BT_BES_CNTL5_ADDR) & ~((uint32_t)0x00010000)) | ((uint32_t)fampmode << 16));
}

__STATIC_FORCEINLINE void bt_bes_cntl5_multifa_mode_0_setf(uint8_t multifamode0)
{
    ASSERT_ERR((((uint32_t)multifamode0 << 24) & ~((uint32_t)0x01000000)) == 0);
    REG_BT_WR(BT_BES_CNTL5_ADDR, (REG_BT_RD(BT_BES_CNTL5_ADDR) & ~((uint32_t)0x01000000)) | ((uint32_t)multifamode0 << 24));
}

__STATIC_FORCEINLINE void bt_bes_cntl5_multifa_mode_1_setf(uint8_t multifamode1)
{
    ASSERT_ERR((((uint32_t)multifamode1 << 25) & ~((uint32_t)0x02000000)) == 0);
    REG_BT_WR(BT_BES_CNTL5_ADDR, (REG_BT_RD(BT_BES_CNTL5_ADDR) & ~((uint32_t)0x02000000)) | ((uint32_t)multifamode1 << 25));
}

__STATIC_FORCEINLINE void bt_bes_cntl5_mode_1_fa_times_setf(uint8_t mode1fatimes)
{
    ASSERT_ERR((((uint32_t)mode1fatimes << 22) & ~((uint32_t)0x00C00000)) == 0);
    REG_BT_WR(BT_BES_CNTL5_ADDR, (REG_BT_RD(BT_BES_CNTL5_ADDR) & ~((uint32_t)0x00C00000)) | ((uint32_t)mode1fatimes << 22));
}

#define BT_BES_CNTLX_ADDR              (BT_CORE_BASE_ADDR + 0xC7C)
#define BT_CAP_SEL_ADDR                (BT_CORE_BASE_ADDR + 0xC50)

/**
 * @brief TRIGREG register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31   reg_epta_freqnum_dr   0
 *     30    reg_relay_arqn_en   0
 *  29:27     reg_cnt_pkt_us_h   0x0
 *     26     reg_rxintr_bt_en   0
 *     25    reg_rxintr_ble_en   0
 *  24:18   reg_2dh5_fa_waitlen   0x0
 *     17     pkt_2dh3bideccen   0
 *     16     pkt_3dh5bideccen   0
 *     15     pkt_2dh5bideccen   0
 *     14             fafreqen   0
 *     11        fa_mt_2dh3_en   0
 *     10        fa_mt_3dh5_en   0
 *     09        fa_mt_2dh5_en   0
 *  08:02   pkt_2dh3_reduce_byte   0x0
 *     00       trig_lock_mode   1
 * </pre>
 */
#define BT_TRIGREG_ADDR                (BT_CORE_BASE_ADDR + 0xC80)


__STATIC_FORCEINLINE void bt_trigreg_reg_cnt_pkt_us_h_setf(uint8_t regcntpktush)
{
    ASSERT_ERR((((uint32_t)regcntpktush << 27) & ~((uint32_t)0x38000000)) == 0);
    REG_BT_WR(BT_TRIGREG_ADDR, (REG_BT_RD(BT_TRIGREG_ADDR) & ~((uint32_t)0x38000000)) | ((uint32_t)regcntpktush << 27));
}

#define BT_TWSBTCLKREG_ADDR            (BT_CORE_BASE_ADDR + 0xC84)

/**
 * @brief BES_PCMCNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:28      reg_pcm_enh_num   0x0
 *  27:20      pcmout_fix_data   0x0
 *     19   pcmout_fix_data_en   0
 *     18       pcmout_test_en   0
 *     17          pcm_enhmode   0
 *     16     reg_enh_pcm_mask   0
 *     15         pcm_fastmode   0
 *  14:08   pcm_esco_frame_num   0x3B
 *     07         pcm_write_en   1
 *  06:00     pcm_fastmode_cnt   0xF
 * </pre>
 */
#define BT_BES_PCMCNTL_ADDR            (BT_CORE_BASE_ADDR + 0xC88)

__STATIC_FORCEINLINE void bt_bes_pcmcntl_pcm_enhmode_setf(uint8_t pcmenhmode)
{
    ASSERT_ERR((((uint32_t)pcmenhmode << 17) & ~((uint32_t)0x00020000)) == 0);
    REG_BT_WR(BT_BES_PCMCNTL_ADDR, (REG_BT_RD(BT_BES_PCMCNTL_ADDR) & ~((uint32_t)0x00020000)) | ((uint32_t)pcmenhmode << 17));
}

__STATIC_FORCEINLINE void bt_bes_pcmcntl_pcm_fastmode_setf(uint8_t pcmfastmode)
{
    ASSERT_ERR((((uint32_t)pcmfastmode << 15) & ~((uint32_t)0x00008000)) == 0);
    REG_BT_WR(BT_BES_PCMCNTL_ADDR, (REG_BT_RD(BT_BES_PCMCNTL_ADDR) & ~((uint32_t)0x00008000)) | ((uint32_t)pcmfastmode << 15));
}

__STATIC_FORCEINLINE void bt_bes_pcmcntl_pcm_esco_frame_num_setf(uint8_t pcmescoframenum)
{
    ASSERT_ERR((((uint32_t)pcmescoframenum << 8) & ~((uint32_t)0x00007F00)) == 0);
    REG_BT_WR(BT_BES_PCMCNTL_ADDR, (REG_BT_RD(BT_BES_PCMCNTL_ADDR) & ~((uint32_t)0x00007F00)) | ((uint32_t)pcmescoframenum << 8));
}
/**
 * @brief BES_FACNTL0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31      reg_forceack_en   0
 *     30    reg_farx_alwayson   1
 *     29       reg_ecc_bid_en   0
 *     28      reg_fasync_mode   1
 *     27   reg_tws_slave_retx_ecc_en   0
 *  26:18      reg_type_ecc_en   0x1FF
 *     17      reg_faac_inv_en   0
 *  16:10   reg_farx_timeout_cnt   0x0
 *     09   reg_farx_timeout_en   0
 *     08   reg_esco_pkt_ecc_en   1
 *  05:03         reg_fa_guard   0x6
 *  02:01   reg_fa_rate_mode_xsco   0x2
 *     00       reg_2m_fa_mode   0
 * </pre>
 */
#define BT_BES_FACNTL0_ADDR            (BT_CORE_BASE_ADDR + 0xC8C)

__STATIC_FORCEINLINE void bt_bes_facntl0_reg_faac_inv_en_setf(uint8_t regfaacinven)
{
    ASSERT_ERR((((uint32_t)regfaacinven << 17) & ~((uint32_t)0x00020000)) == 0);
    REG_BT_WR(BT_BES_FACNTL0_ADDR, (REG_BT_RD(BT_BES_FACNTL0_ADDR) & ~((uint32_t)0x00020000)) | ((uint32_t)regfaacinven << 17));
}

__STATIC_FORCEINLINE void bt_bes_facntl0_reg_farx_alwayson_setf(uint8_t regfarxalwayson)
{
    ASSERT_ERR((((uint32_t)regfarxalwayson << 30) & ~((uint32_t)0x40000000)) == 0);
    REG_BT_WR(BT_BES_FACNTL0_ADDR, (REG_BT_RD(BT_BES_FACNTL0_ADDR) & ~((uint32_t)0x40000000)) | ((uint32_t)regfarxalwayson << 30));
}

__STATIC_FORCEINLINE void bt_bes_facntl0_reg__2m_fa_mode_setf(uint8_t reg2mfamode)
{
    ASSERT_ERR((((uint32_t)reg2mfamode << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_BT_WR(BT_BES_FACNTL0_ADDR, (REG_BT_RD(BT_BES_FACNTL0_ADDR) & ~((uint32_t)0x00000001)) | ((uint32_t)reg2mfamode << 0));
}

__STATIC_FORCEINLINE void bt_bes_facntl0_reg_fasync_mode_setf(uint8_t regfasyncmode)
{
    ASSERT_ERR((((uint32_t)regfasyncmode << 28) & ~((uint32_t)0x10000000)) == 0);
    REG_BT_WR(BT_BES_FACNTL0_ADDR, (REG_BT_RD(BT_BES_FACNTL0_ADDR) & ~((uint32_t)0x10000000)) | ((uint32_t)regfasyncmode << 28));
}
/**
 * @brief BES_FACNTL1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31         reg_fa_sw_dr   0
 *  30:24       reg_fa_rxwinsz   0x1A
 *     23   reg_pkt_ecc_len_dr   0
 *  22:16      reg_pkt_ecc_len   0x1A
 *  15:05    reg_ecc_frame_len   0x0
 *     04   reg_ecc_frame_len_dr   0
 *     03   reg_low_txpwr_sel_en   0
 *  02:00           reg_t_mode   0x0
 * </pre>
 */
#define BT_BES_FACNTL1_ADDR            (BT_CORE_BASE_ADDR + 0xC90)

__STATIC_FORCEINLINE void bt_bes_facntl1_reg_fa_rxwinsz_setf(uint8_t regfarxwinsz)
{
    ASSERT_ERR((((uint32_t)regfarxwinsz << 24) & ~((uint32_t)0x7F000000)) == 0);
    REG_BT_WR(BT_BES_FACNTL1_ADDR, (REG_BT_RD(BT_BES_FACNTL1_ADDR) & ~((uint32_t)0x7F000000)) | ((uint32_t)regfarxwinsz << 24));
}

#define BT_BES_TG_CLKNCNT_ADDR         (BT_CORE_BASE_ADDR + 0xC94)
#define BT_BES_TG_FINECNT_ADDR         (BT_CORE_BASE_ADDR + 0xC98)
#define BT_BES_TG_CLKNCNT1_ADDR        (BT_CORE_BASE_ADDR + 0xD08)
#define BT_BES_TG_FINECNT1_ADDR        (BT_CORE_BASE_ADDR + 0xD0C)
#define BT_BES_TG_CLKNCNT2_ADDR        (BT_CORE_BASE_ADDR + 0xD10)
#define BT_BES_TG_FINECNT2_ADDR        (BT_CORE_BASE_ADDR + 0xD14)
#define BT_BES_TG_CLKNCNT3_ADDR        (BT_CORE_BASE_ADDR + 0xD18)
#define BT_BES_TG_FINECNT3_ADDR        (BT_CORE_BASE_ADDR + 0xD1C)
#define BT_BES_SYNC_INTR_ADDR          (BT_CORE_BASE_ADDR + 0xD28)
#define BT_BES_TOG_CNTL_ADDR           (BT_CORE_BASE_ADDR + 0xD70)
#define BT_BES_PCM_DF_REG1_ADDR        (BT_CORE_BASE_ADDR + 0xCA0)
#define BT_BES_PCM_DF_REG_ADDR         (BT_CORE_BASE_ADDR + 0xC9C)
#define BT_BES_FA_SWREG0_ADDR          (BT_CORE_BASE_ADDR + 0xCC4)
#define BT_BES_FA_SWREG1_ADDR          (BT_CORE_BASE_ADDR + 0xCC8)
#define BT_E_SCOMUTECNTL_0_ADDR        (BT_CORE_BASE_ADDR + 0x614)
#define BT_E_SCOTRCNTL_ADDR            (BT_CORE_BASE_ADDR + 0x624)
#define BT_E_SCOMUTECNTL_1_ADDR        (BT_CORE_BASE_ADDR + 0x634)
#define BT_E_SCOCURRENTTXPTR_ADDR      (BT_CORE_BASE_ADDR + 0x618)
#define BT_AUDIOCNTL0_ADDR             (BT_CORE_BASE_ADDR + 0x670)
#define BT_AUDIOCNTL1_ADDR             (BT_CORE_BASE_ADDR + 0x674)
#define BT_PCMGENCNTL_ADDR             (BT_CORE_BASE_ADDR + 0x680)

/**
 * @brief PCMPHYSCNTL1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31               CLKINV   0
 *  23:16          PCMCLKLIMIT   0x0
 *  08:00            PCMCLKVAL   0x0
 * </pre>
 */

#define BT_PCMPHYSCNTL1_ADDR           (BT_CORE_BASE_ADDR + 0x688)
#define BT_RADIOPWRUPDN_ADDR           (BT_CORE_BASE_ADDR + 0x48C)
#define BT_RADIOTXRXTIM_ADDR           (BT_CORE_BASE_ADDR + 0x490)

/**
 * @brief BES_ENHPCM_CNTL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31     SLOT_AUTO_SEL_EN   0
 *  29:27           afh_rxgain   0x0
 *  26:23   bidecc_wait_offset   0x0
 *  15:06          bid_ecc_len   0x0
 *     05           bid_ecc_en   0
 *  04:01   pcm_enhmode2_slot_cnt   0x5
 *     00     reg_pcm_enhmode2   0
 * </pre>
 */

#define BT_BES_ENHPCM_CNTL_ADDR        (BT_CORE_BASE_ADDR + 0xCBC)

__STATIC_FORCEINLINE void bt_bes_enhpcm_cntl_afh_rxgain_setf(uint8_t afhrxgain)
{
    ASSERT_ERR((((uint32_t)afhrxgain << 27) & ~((uint32_t)0x38000000)) == 0);
    REG_BT_WR(BT_BES_ENHPCM_CNTL_ADDR, (REG_BT_RD(BT_BES_ENHPCM_CNTL_ADDR) & ~((uint32_t)0x38000000)) | ((uint32_t)afhrxgain << 27));
}

#define BT_CURRENTRXDESCPTR_ADDR       (BT_CORE_BASE_ADDR + 0x428)
#define BT_AFH_MONITOR_ADDR            (BT_CORE_BASE_ADDR + 0xD54)
#define IP_SLOTCLK_ADDR                (BT_CORE_BASE_ADDR + 0x100)
#define IP_ISOCNTSAMP_ADDR             (BT_CORE_BASE_ADDR + 0x9C4)

/**
 * @brief RADIOCNTL3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:30           RXRATE3CFG   0x3
 *  29:28           RXRATE2CFG   0x2
 *  27:26           RXRATE1CFG   0x1
 *  25:24           RXRATE0CFG   0x0
 *  22:20         GETRSSIDELAY   0x4
 *     18       RXSYNC_ROUTING   0
 *  17:16          RXVALID_BEH   0x0
 *  15:14           TXRATE3CFG   0x3
 *  13:12           TXRATE2CFG   0x2
 *  11:10           TXRATE1CFG   0x1
 *  09:08           TXRATE0CFG   0x0
 *  01:00          TXVALID_BEH   0x0
 * </pre>
 */
#define BLE_RADIOCNTL3_ADDR   (BT_CORE_BASE_ADDR + 0x87C)
/**
 * @brief RADIOPWRUPDN0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24       SYNC_POSITION0   0x0
 *  23:16             RXPWRUP0   0x0
 *  14:08             TXPWRDN0   0x0
 *  07:00             TXPWRUP0   0x0
 * </pre>
 */
#define BLE_RADIOPWRUPDN0_ADDR   (BT_CORE_BASE_ADDR + 0x880)
/**
 * @brief RADIOPWRUPDN1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24       SYNC_POSITION1   0x0
 *  23:16             RXPWRUP1   0x0
 *  14:08             TXPWRDN1   0x0
 *  07:00             TXPWRUP1   0x0
 * </pre>
 */
#define BLE_RADIOPWRUPDN1_ADDR   (BT_CORE_BASE_ADDR + 0x884)
/**
 * @brief RADIOPWRUPDN2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24       SYNC_POSITION2   0x0
 *  23:16             RXPWRUP2   0x0
 *  14:08             TXPWRDN2   0x0
 *  07:00             TXPWRUP2   0x0
 * </pre>
 */
#define BLE_RADIOPWRUPDN2_ADDR   (BT_CORE_BASE_ADDR + 0x888)
/**
 * @brief RADIOPWRUPDN3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  14:08             TXPWRDN3   0x0
 *  07:00             TXPWRUP3   0x0
 * </pre>
 */
#define BLE_RADIOPWRUPDN3_ADDR   (BT_CORE_BASE_ADDR + 0x88c)
/**
 * @brief RADIOTXRXTIM0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  22:16            RFRXTMDA0   0x0
 *  14:08           RXPATHDLY0   0x0
 *  06:00           TXPATHDLY0   0x0
 * </pre>
 */
#define BLE_RADIOTXRXTIM0_ADDR   (BT_CORE_BASE_ADDR + 0x890)
/**
 * @brief RADIOTXRXTIM1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  22:16            RFRXTMDA1   0x0
 *  14:08           RXPATHDLY1   0x0
 *  06:00           TXPATHDLY1   0x0
 * </pre>
 */
#define BLE_RADIOTXRXTIM1_ADDR   (BT_CORE_BASE_ADDR + 0x894)
/**
 * @brief RADIOTXRXTIM2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24      RXFLUSHPATHDLY2   0x0
 *  23:16            RFRXTMDA2   0x0
 *  15:08           RXPATHDLY2   0x0
 *  06:00           TXPATHDLY2   0x0
 * </pre>
 */
#define BLE_RADIOTXRXTIM2_ADDR   (BT_CORE_BASE_ADDR + 0x898)
/**
 * @brief RADIOTXRXTIM3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24      RXFLUSHPATHDLY3   0x0
 *  22:16            RFRXTMDA3   0x0
 *  06:00           TXPATHDLY3   0x0
 * </pre>
 */
#define BLE_RADIOTXRXTIM3_ADDR   (BT_CORE_BASE_ADDR + 0x89C)

/**
 * @brief BES_TESTMODE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00          bt_testmode   0x0
 * </pre>
 */
#define BT_BES_TESTMODE_ADDR   (BT_CORE_BASE_ADDR + 0xC00)

__STATIC_FORCEINLINE void bt_bes_testmode_set(uint32_t value)
{
    REG_BT_WR(BT_BES_TESTMODE_ADDR, value);
}
/**
 * @brief BES_CNTL4 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:29         sbc_link_sel   0x0
 *     28         sbc_type_sel   0
 *  27:24        reg_sbc_type2   0x0
 *  23:20        reg_sbc_type1   0x0
 *  19:16        reg_sbc_type0   0x0
 *     15         reg_sbc_edr2   0
 *     14         reg_sbc_edr1   0
 *     13         reg_sbc_edr0   0
 *  06:00       reg_rampdn_cnt   0x0
 * </pre>
 */
#define BT_BES_CNTL4_ADDR   (BT_CORE_BASE_ADDR + 0xC14)

__STATIC_FORCEINLINE void bt_core_cntl4_reg_rampdn_cnt_setf(uint8_t regrampdncnt)
{
    REG_BT_WR(BT_BES_CNTL4_ADDR, (REG_BT_RD(BT_BES_CNTL4_ADDR) & ~((uint32_t)0x0000007F)) | ((uint32_t)regrampdncnt << 0));
}
/**
 * @brief SWAGC_RX_BT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     30   bt_sniff_to_fix_en   0
 *  29:27    new_swagc_rxidx10   0x0
 *  26:24     new_swagc_rxidx9   0x0
 *  23:21     new_swagc_rxidx8   0x0
 *  20:18     new_swagc_rxidx7   0x0
 *  17:15     new_swagc_rxidx6   0x0
 *  14:12     new_swagc_rxidx5   0x0
 *  11:09     new_swagc_rxidx4   0x0
 *  08:06     new_swagc_rxidx3   0x0
 *  05:03     new_swagc_rxidx2   0x0
 *  02:00     new_swagc_rxidx1   0x0
 * </pre>
 */
#define BT_SWAGC_RX_ADDR   (BT_CORE_BASE_ADDR + 0xC38)

__STATIC_FORCEINLINE void bt_core_sniff_to_fix_en_setf(uint8_t btsnifftofixen)
{
    REG_BT_WR(BT_SWAGC_RX_ADDR, (REG_BT_RD(BT_SWAGC_RX_ADDR) & ~((uint32_t)0x40000000)) | ((uint32_t)btsnifftofixen << 30));
}

/**
 * @brief AGC_BLE_RX register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:30       dig_i2v_bypass   0x0
 *  29:27   new_swagc_rxidx10_ble   0x0
 *  26:24   new_swagc_rxidx9_ble   0x0
 *  23:21   new_swagc_rxidx8_ble   0x0
 *  20:18   new_swagc_rxidx7_ble   0x0
 *  17:15   new_swagc_rxidx6_ble   0x0
 *  14:12   new_swagc_rxidx5_ble   0x0
 *  11:09   new_swagc_rxidx4_ble   0x0
 *  08:06   new_swagc_rxidx3_ble   0x0
 *  05:03   new_swagc_rxidx2_ble   0x0
 *  02:00   new_swagc_rxidx1_ble   0x0
 * </pre>
 */
#define BT_AGC_BLE_RX_ADDR   (BT_CORE_BASE_ADDR + 0xC3C)

__STATIC_FORCEINLINE void bt_agc_ble_rx_dig_i_2v_bypass_setf(uint8_t digi2vbypass)
{
    ASSERT_ERR((((uint32_t)digi2vbypass << 30) & ~((uint32_t)0xC0000000)) == 0);
    REG_BT_WR(BT_AGC_BLE_RX_ADDR, (REG_BT_RD(BT_AGC_BLE_RX_ADDR) & ~((uint32_t)0xC0000000)) | ((uint32_t)digi2vbypass << 30));
}

/**
 * @brief HDT_4M_TXPATHDLY register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     23       mode_timing_en   0
 *  22:16         bt_txpwrdn4m   0x0
 *  15:08         bt_txpwrup4m   0x0
 *  06:00       bt_txpathdly4m   0x0
 * </pre>
 */
#define BT_HDT_4M_TXPATHDLY_ADDR   (BT_CORE_BASE_ADDR + 0xC60)
__STATIC_FORCEINLINE void btdrv_hdt_4m_txpathdly_pack(uint8_t modetimingen, uint8_t bttxpwrdn4m, uint8_t bttxpwrup4m, uint8_t bttxpathdly4m)
{
    ASSERT_ERR((((uint32_t)modetimingen << 23) & ~((uint32_t)0x00800000)) == 0);
    ASSERT_ERR((((uint32_t)bttxpwrdn4m << 16) & ~((uint32_t)0x007F0000)) == 0);
    ASSERT_ERR((((uint32_t)bttxpwrup4m << 8) & ~((uint32_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint32_t)bttxpathdly4m << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_BT_WR(BT_HDT_4M_TXPATHDLY_ADDR,  ((uint32_t)modetimingen << 23) | ((uint32_t)bttxpwrdn4m << 16) | ((uint32_t)bttxpwrup4m << 8) | ((uint32_t)bttxpathdly4m << 0));
}

/**
 * @brief HDT_4M_RXPATHDLY register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:16         bt_syncpos4m   0x0
 *  15:08         bt_rxpwrup4m   0x0
 *  06:00       bt_rxpathdly4m   0x0
 * </pre>
 */
#define BT_HDT_4M_RXPATHDLY_ADDR   (BT_CORE_BASE_ADDR + 0xC64)
__STATIC_FORCEINLINE void btdrv_hdt_4m_rxpathdly_pack(uint8_t btsyncpos4m, uint8_t btrxpwrup4m, uint8_t btrxpathdly4m)
{
    ASSERT_ERR((((uint32_t)btsyncpos4m << 16) & ~((uint32_t)0x00FF0000)) == 0);
    ASSERT_ERR((((uint32_t)btrxpwrup4m << 8) & ~((uint32_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint32_t)btrxpathdly4m << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_BT_WR(BT_HDT_4M_RXPATHDLY_ADDR,  ((uint32_t)btsyncpos4m << 16) | ((uint32_t)btrxpwrup4m << 8) | ((uint32_t)btrxpathdly4m << 0));
}


/**
 * @brief HDT_2M_TXPATHDLY register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  22:16         bt_txpwrdn2m   0x0
 *  15:08         bt_txpwrup2m   0x0
 *  06:00       bt_txpathdly2m   0x0
 * </pre>
 */
#define BT_HDT_2M_TXPATHDLY_ADDR   (BT_CORE_BASE_ADDR + 0xC68)
__STATIC_FORCEINLINE void btdrv_hdt_2m_txpathdly_pack(uint8_t bttxpwrdn2m, uint8_t bttxpwrup2m, uint8_t bttxpathdly2m)
{
    ASSERT_ERR((((uint32_t)bttxpwrdn2m << 16) & ~((uint32_t)0x007F0000)) == 0);
    ASSERT_ERR((((uint32_t)bttxpwrup2m << 8) & ~((uint32_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint32_t)bttxpathdly2m << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_BT_WR(BT_HDT_2M_TXPATHDLY_ADDR,  ((uint32_t)bttxpwrdn2m << 16) | ((uint32_t)bttxpwrup2m << 8) | ((uint32_t)bttxpathdly2m << 0));
}

/**
 * @brief HDT_2M_RXPATHDLY register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:16         bt_syncpos2m   0x0
 *  15:08         bt_rxpwrup2m   0x0
 *  06:00       bt_rxpathdly2m   0x0
 * </pre>
 */
#define BT_HDT_2M_RXPATHDLY_ADDR   (BT_CORE_BASE_ADDR + 0xC6C)
__STATIC_FORCEINLINE void btdrv_hdt_2m_rxpathdly_pack(uint8_t btsyncpos2m, uint8_t btrxpwrup2m, uint8_t btrxpathdly2m)
{
    ASSERT_ERR((((uint32_t)btsyncpos2m << 16) & ~((uint32_t)0x00FF0000)) == 0);
    ASSERT_ERR((((uint32_t)btrxpwrup2m << 8) & ~((uint32_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint32_t)btrxpathdly2m << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_BT_WR(BT_HDT_2M_RXPATHDLY_ADDR,  ((uint32_t)btsyncpos2m << 16) | ((uint32_t)btrxpwrup2m << 8) | ((uint32_t)btrxpathdly2m << 0));
}
/**
 * @brief BES_CNTL15 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     30         reg_relay_en   0
 *  29:22        reg_cnt_relay   0x0
 *  19:10               rf_spi   0x0
 *  07:04     reg_radd_cnt_rec   0x0
 *  03:00     reg_wadd_cnt_rec   0x0
 * </pre>
 */
#define BT_BES_CNTL15_ADDR   (BT_CORE_BASE_ADDR + 0xD04)
/**
 * @brief MTK_MHDT_0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     30   reg_trigger_irq_en   0
 *     28   reg_soft_rst_irq_en   0
 *     26   reg_mhdt_ble_fa_dsb   0
 *     25      reg_mhdt_fa_dsb   0
 *  24:23       ble_txrate4cfg   0x0
 *  22:16    reg_ble_txpathdly   0x0
 *  15:08    reg_ble_txpwrup4m   0x0
 *  06:00      reg_ble_pwrdn4m   0x0
 * </pre>
 */
#define BLE_MTK_MHDT_0_ADDR   (BT_CORE_BASE_ADDR + 0xD78)
__STATIC_FORCEINLINE void ble_mtk_mhdt_0_pack(uint8_t regtriggerirqen, uint8_t regsoftrstirqen, uint8_t regmhdtblefadsb, uint8_t regmhdtfadsb, uint8_t bletxrate4cfg, uint8_t regbletxpathdly, uint8_t regbletxpwrup4m, uint8_t regblepwrdn4m)
{
    ASSERT_ERR((((uint32_t)regtriggerirqen << 30) & ~((uint32_t)0x40000000)) == 0);
    ASSERT_ERR((((uint32_t)regsoftrstirqen << 28) & ~((uint32_t)0x10000000)) == 0);
    ASSERT_ERR((((uint32_t)regmhdtblefadsb << 26) & ~((uint32_t)0x04000000)) == 0);
    ASSERT_ERR((((uint32_t)regmhdtfadsb << 25) & ~((uint32_t)0x02000000)) == 0);
    ASSERT_ERR((((uint32_t)bletxrate4cfg << 23) & ~((uint32_t)0x01800000)) == 0);
    ASSERT_ERR((((uint32_t)regbletxpathdly << 16) & ~((uint32_t)0x007F0000)) == 0);
    ASSERT_ERR((((uint32_t)regbletxpwrup4m << 8) & ~((uint32_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint32_t)regblepwrdn4m << 0) & ~((uint32_t)0x0000007F)) == 0);
    REG_BT_WR(BLE_MTK_MHDT_0_ADDR,  ((uint32_t)regtriggerirqen << 30) | ((uint32_t)regsoftrstirqen << 28) | ((uint32_t)regmhdtblefadsb << 26) | ((uint32_t)regmhdtfadsb << 25) | ((uint32_t)bletxrate4cfg << 23) | ((uint32_t)regbletxpathdly << 16) | ((uint32_t)regbletxpwrup4m << 8) | ((uint32_t)regblepwrdn4m << 0));
}

/**
 * @brief MTK_HDTBW register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  25:24         ble_rate4cfg   0x0
 *  23:16   eg_ble_rxpathdelay4m   0x0
 *  15:08    reg_ble_rxpwrup4m   0x0
 *  07:00   reg_ble_sync_position4m   0x0
 * </pre>
 */
#define BLE_MTK_HDTBW_ADDR   (BT_CORE_BASE_ADDR + 0xD7C)
__STATIC_FORCEINLINE void ble_mtk_hdtbw_pack(uint8_t blerate4cfg, uint8_t egblerxpathdelay4m, uint8_t regblerxpwrup4m, uint8_t regblesyncposition4m)
{
    ASSERT_ERR((((uint32_t)blerate4cfg << 24) & ~((uint32_t)0x03000000)) == 0);
    ASSERT_ERR((((uint32_t)egblerxpathdelay4m << 16) & ~((uint32_t)0x00FF0000)) == 0);
    ASSERT_ERR((((uint32_t)regblerxpwrup4m << 8) & ~((uint32_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint32_t)regblesyncposition4m << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_BT_WR(BLE_MTK_HDTBW_ADDR,  ((uint32_t)blerate4cfg << 24) | ((uint32_t)egblerxpathdelay4m << 16) | ((uint32_t)regblerxpwrup4m << 8) | ((uint32_t)regblesyncposition4m << 0));
}

/**
 * @brief ISOCNT_FREERUN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00              iso_cnt   0x0
 * </pre>
 */
#define BT_ISOCNT_FREERUN_ADDR   (BT_CORE_BASE_ADDR + 0xE78)

__STATIC_FORCEINLINE uint32_t bt_isocnt_freerun_get(void)
{
    return REG_BT_RD(BT_ISOCNT_FREERUN_ADDR);
}

/**
 * @brief AFH_MONITOR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  29:24              win_num   0x0
 *  22:16          win_spacing   0x0
 *  14:08             win_size   0x0
 *  03:00          bt_prio_thd   0x0
 * </pre>
 */
//#define BT_AFH_MONITOR_ADDR   0xD0220D54
#define BT_AFH_MONITOR_OFFSET 0x00000154
#define BT_AFH_MONITOR_INDEX  0x00000055
#define BT_AFH_MONITOR_RESET  0x00000000

__STATIC_FORCEINLINE uint32_t bt_afh_monitor_get(void)
{
    return REG_BT_RD(BT_AFH_MONITOR_ADDR);
}

__STATIC_FORCEINLINE void bt_afh_monitor_set(uint32_t value)
{
    REG_BT_WR(BT_AFH_MONITOR_ADDR, value);
}

// field definitions
#define BT_WIN_NUM_MASK       ((uint32_t)0x3F000000)
#define BT_WIN_NUM_LSB        24
#define BT_WIN_NUM_WIDTH      ((uint32_t)0x00000006)
#define BT_WIN_SPACING_MASK   ((uint32_t)0x007F0000)
#define BT_WIN_SPACING_LSB    16
#define BT_WIN_SPACING_WIDTH  ((uint32_t)0x00000007)
#define BT_WIN_SIZE_MASK      ((uint32_t)0x00007F00)
#define BT_WIN_SIZE_LSB       8
#define BT_WIN_SIZE_WIDTH     ((uint32_t)0x00000007)
#define BT_BT_PRIO_THD_MASK   ((uint32_t)0x0000000F)
#define BT_BT_PRIO_THD_LSB    0
#define BT_BT_PRIO_THD_WIDTH  ((uint32_t)0x00000004)

#define BT_WIN_NUM_RST        0x0
#define BT_WIN_SPACING_RST    0x0
#define BT_WIN_SIZE_RST       0x0
#define BT_BT_PRIO_THD_RST    0x0

__STATIC_FORCEINLINE void bt_afh_monitor_pack(uint8_t winnum, uint8_t winspacing, uint8_t winsize, uint8_t btpriothd)
{
    ASSERT_ERR((((uint32_t)winnum << 24) & ~((uint32_t)0x3F000000)) == 0);
    ASSERT_ERR((((uint32_t)winspacing << 16) & ~((uint32_t)0x007F0000)) == 0);
    ASSERT_ERR((((uint32_t)winsize << 8) & ~((uint32_t)0x00007F00)) == 0);
    ASSERT_ERR((((uint32_t)btpriothd << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_BT_WR(BT_AFH_MONITOR_ADDR,  ((uint32_t)winnum << 24) | ((uint32_t)winspacing << 16) | ((uint32_t)winsize << 8) | ((uint32_t)btpriothd << 0));
}

__STATIC_FORCEINLINE void bt_afh_monitor_unpack(uint8_t* winnum, uint8_t* winspacing, uint8_t* winsize, uint8_t* btpriothd)
{
    uint32_t localVal = REG_BT_RD(BT_AFH_MONITOR_ADDR);

    *winnum = (localVal & ((uint32_t)0x3F000000)) >> 24;
    *winspacing = (localVal & ((uint32_t)0x007F0000)) >> 16;
    *winsize = (localVal & ((uint32_t)0x00007F00)) >> 8;
    *btpriothd = (localVal & ((uint32_t)0x0000000F)) >> 0;
}

__STATIC_FORCEINLINE uint8_t bt_afh_monitor_win_num_getf(void)
{
    uint32_t localVal = REG_BT_RD(BT_AFH_MONITOR_ADDR);
    return ((localVal & ((uint32_t)0x3F000000)) >> 24);
}

__STATIC_FORCEINLINE void bt_afh_monitor_win_num_setf(uint8_t winnum)
{
    ASSERT_ERR((((uint32_t)winnum << 24) & ~((uint32_t)0x3F000000)) == 0);
    REG_BT_WR(BT_AFH_MONITOR_ADDR, (REG_BT_RD(BT_AFH_MONITOR_ADDR) & ~((uint32_t)0x3F000000)) | ((uint32_t)winnum << 24));
}

__STATIC_FORCEINLINE uint8_t bt_afh_monitor_win_spacing_getf(void)
{
    uint32_t localVal = REG_BT_RD(BT_AFH_MONITOR_ADDR);
    return ((localVal & ((uint32_t)0x007F0000)) >> 16);
}

__STATIC_FORCEINLINE void bt_afh_monitor_win_spacing_setf(uint8_t winspacing)
{
    ASSERT_ERR((((uint32_t)winspacing << 16) & ~((uint32_t)0x007F0000)) == 0);
    REG_BT_WR(BT_AFH_MONITOR_ADDR, (REG_BT_RD(BT_AFH_MONITOR_ADDR) & ~((uint32_t)0x007F0000)) | ((uint32_t)winspacing << 16));
}

__STATIC_FORCEINLINE uint8_t bt_afh_monitor_win_size_getf(void)
{
    uint32_t localVal = REG_BT_RD(BT_AFH_MONITOR_ADDR);
    return ((localVal & ((uint32_t)0x00007F00)) >> 8);
}

__STATIC_FORCEINLINE void bt_afh_monitor_win_size_setf(uint8_t winsize)
{
    ASSERT_ERR((((uint32_t)winsize << 8) & ~((uint32_t)0x00007F00)) == 0);
    REG_BT_WR(BT_AFH_MONITOR_ADDR, (REG_BT_RD(BT_AFH_MONITOR_ADDR) & ~((uint32_t)0x00007F00)) | ((uint32_t)winsize << 8));
}

__STATIC_FORCEINLINE uint8_t bt_afh_monitor_bt_prio_thd_getf(void)
{
    uint32_t localVal = REG_BT_RD(BT_AFH_MONITOR_ADDR);
    return ((localVal & ((uint32_t)0x0000000F)) >> 0);
}

__STATIC_FORCEINLINE void bt_afh_monitor_bt_prio_thd_setf(uint8_t btpriothd)
{
    ASSERT_ERR((((uint32_t)btpriothd << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_BT_WR(BT_AFH_MONITOR_ADDR, (REG_BT_RD(BT_AFH_MONITOR_ADDR) & ~((uint32_t)0x0000000F)) | ((uint32_t)btpriothd << 0));
}
#endif//__BT_1306P_REG_MAP_H__
