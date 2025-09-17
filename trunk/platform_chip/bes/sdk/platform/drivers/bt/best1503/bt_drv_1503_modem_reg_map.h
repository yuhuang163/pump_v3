/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "bt_drv.h"

#ifndef ASSERT_ERR
#define ASSERT_ERR(cond)                             { if (!(cond)) { BT_DRV_TRACE(2,"line is %d file is %s", __LINE__, __FILE__); } }
#endif

#define REG_RPL_WR(reg, mask, shift, v)\
                                                do{ \
                                                    volatile unsigned int tmp = *(volatile unsigned int *)(reg); \
                                                    tmp &= ~mask; \
                                                    tmp |= (v<<shift); \
                                                    *(volatile unsigned int *)(reg) = tmp; \
                                                }while(0)

/**
 * @brief BW2M_RC_IF_VALUE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00     bw2m_rc_if_value   0x0
 * </pre>
 */
#define BESMDM_BW2M_RC_IF_VALUE_ADDR    0xD03500D0
#define BESMDM_BW_2M_RC_IF_VALUE_MASK   ((uint32_t)0xFFFFFFFF)
#define BESMDM_BW_2M_RC_IF_VALUE_LSB    0
__STATIC_FORCEINLINE void besmdm_bw_2m_rc_if_value_setf(uint32_t bw2mrcifvalue)
{
    ASSERT_ERR((((uint32_t)bw2mrcifvalue << 0) & ~((uint32_t)0xFFFFFFFF)) == 0);
    REG_RPL_WR(BESMDM_BW2M_RC_IF_VALUE_ADDR, BESMDM_BW_2M_RC_IF_VALUE_MASK, BESMDM_BW_2M_RC_IF_VALUE_LSB, bw2mrcifvalue);
}

/**
 * @brief BW2M_RC_STEP register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  24:00         bw2m_rc_step   0x0
 * </pre>
 */
#define BESMDM_BW2M_RC_STEP_ADDR    0xD035014C
#define BESMDM_BW_2M_RC_STEP_MASK   ((uint32_t)0x01FFFFFF)
#define BESMDM_BW_2M_RC_STEP_LSB    0
__STATIC_FORCEINLINE void besmdm_bw_2m_rc_step_setf(uint32_t bw2mrcstep)
{
    ASSERT_ERR((((uint32_t)bw2mrcstep << 0) & ~((uint32_t)0x01FFFFFF)) == 0);
    REG_RPL_WR(BESMDM_BW2M_RC_STEP_ADDR, BESMDM_BW_2M_RC_STEP_MASK, BESMDM_BW_2M_RC_STEP_LSB, bw2mrcstep);
}

/**
 * @brief BW2M_RC_CH_STEP register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  19:00      bw2m_rc_ch_step   0x0
 * </pre>
 */
#define BESMDM_BW2M_RC_CH_STEP_ADDR   0xD0350154
#define BESMDM_BW_2M_RC_CH_STEP_MASK   ((uint32_t)0x000FFFFF)
#define BESMDM_BW_2M_RC_CH_STEP_LSB    0
__STATIC_FORCEINLINE void besmdm_bw_2m_rc_ch_step_setf(uint32_t bw2mrcchstep)
{
    ASSERT_ERR((((uint32_t)bw2mrcchstep << 0) & ~((uint32_t)0x000FFFFF)) == 0);
    REG_RPL_WR(BESMDM_BW2M_RC_CH_STEP_ADDR, BESMDM_BW_2M_RC_CH_STEP_MASK, BESMDM_BW_2M_RC_CH_STEP_LSB, bw2mrcchstep);
}

/**
 * @brief RX_ADC register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  01:00    RX_ADC_CLOCK_RATE   0x0
 * </pre>
 */
#define BESMDM_RX_ADC_ADDR              0xD035015C
#define BESMDM_RX_ADC_CLOCK_RATE_MASK   ((uint32_t)0x00000003)
#define BESMDM_RX_ADC_CLOCK_RATE_LSB    0
__STATIC_FORCEINLINE void besmdm_rx_adc_clock_rate_setf(uint8_t rxadcclockrate)
{
    ASSERT_ERR((((uint32_t)rxadcclockrate << 0) & ~((uint32_t)0x00000003)) == 0);
    REG_RPL_WR(BESMDM_RX_ADC_ADDR, BESMDM_RX_ADC_CLOCK_RATE_MASK, BESMDM_RX_ADC_CLOCK_RATE_LSB, rxadcclockrate);
}

/**
 * @brief RX_IQ_SWAP register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:16                rc_if   0x0
 *     15        gfsk_bits_out   0
 *     10        bt_psd_avg_en   0
 *     02           rx_iq_swap   0
 * </pre>
 */
#define BESMDM_RX_IQ_SWAP_ADDR   0xD0350240
#define BESMDM_RC_IF_MASK           ((uint32_t)0xFFFF0000)
#define BESMDM_RC_IF_LSB            16
#define BESMDM_GFSK_BITS_OUT_BIT    ((uint32_t)0x00008000)
#define BESMDM_GFSK_BITS_OUT_POS    15
#define BESMDM_BT_PSD_AVG_EN_BIT    ((uint32_t)0x00000400)
#define BESMDM_BT_PSD_AVG_EN_POS    10
#define BESMDM_RX_IQ_SWAP_BIT       ((uint32_t)0x00000004)
#define BESMDM_RX_IQ_SWAP_POS       2
__STATIC_FORCEINLINE void besmdm_rx_iq_swap_pack(uint16_t rcif, uint8_t gfskbitsout, uint8_t btpsdavgen, uint8_t rxiqswap)
{
    ASSERT_ERR((((uint32_t)rcif << 16) & ~((uint32_t)0xFFFF0000)) == 0);
    ASSERT_ERR((((uint32_t)gfskbitsout << 15) & ~((uint32_t)0x00008000)) == 0);
    ASSERT_ERR((((uint32_t)btpsdavgen << 10) & ~((uint32_t)0x00000400)) == 0);
    ASSERT_ERR((((uint32_t)rxiqswap << 2) & ~((uint32_t)0x00000004)) == 0);
    REG_RPL_WR(BESMDM_RX_IQ_SWAP_ADDR, BESMDM_RC_IF_MASK, BESMDM_RC_IF_LSB, rcif);
    REG_RPL_WR(BESMDM_RX_IQ_SWAP_ADDR, BESMDM_GFSK_BITS_OUT_BIT, BESMDM_GFSK_BITS_OUT_POS, gfskbitsout);
    REG_RPL_WR(BESMDM_RX_IQ_SWAP_ADDR, BESMDM_BT_PSD_AVG_EN_BIT, BESMDM_BT_PSD_AVG_EN_POS, btpsdavgen);
    REG_RPL_WR(BESMDM_RX_IQ_SWAP_ADDR, BESMDM_RX_IQ_SWAP_BIT, BESMDM_RX_IQ_SWAP_POS, rxiqswap);
}

/**
 * @brief RX_RATE_CONVERTER register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  24:00    rx_rate_converter   0x0
 * </pre>
 */
#define BESMDM_RX_RATE_CONVERTER_ADDR   0xD0350248
#define BESMDM_MDLL_CLK_SEL_BIT         ((uint32_t)0x80000000)
#define BESMDM_MDLL_CLK_SEL_POS         31
#define BESMDM_RX_RATE_CONVERTER_MASK   ((uint32_t)0x01FFFFFF)
#define BESMDM_RX_RATE_CONVERTER_LSB    0
__STATIC_FORCEINLINE void besmdm_rx_rate_converter_pack(uint8_t mdllclksel, uint32_t rxrateconverter)
{
    ASSERT_ERR((((uint32_t)mdllclksel << 31) & ~((uint32_t)0x80000000)) == 0);
    ASSERT_ERR((((uint32_t)rxrateconverter << 0) & ~((uint32_t)0x01FFFFFF)) == 0);
    REG_RPL_WR(BESMDM_RX_RATE_CONVERTER_ADDR,  BESMDM_MDLL_CLK_SEL_BIT, BESMDM_MDLL_CLK_SEL_POS, mdllclksel);
    REG_RPL_WR(BESMDM_RX_RATE_CONVERTER_ADDR,  BESMDM_RX_RATE_CONVERTER_MASK, BESMDM_RX_RATE_CONVERTER_LSB, rxrateconverter);
}

/**
 * @brief GFSK_DSG_DEN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  03:00         gfsk_dsg_den   0x0
 * </pre>
 */
#define BESMDM_GFSK_DSG_DEN_ADDR   0xD0350300
#define BESMDM_GFSK_DSG_DEN_MASK   ((uint32_t)0x0000000F)
#define BESMDM_GFSK_DSG_DEN_LSB    0
__STATIC_FORCEINLINE void besmdm_gfsk_dsg_den_setf(uint8_t gfskdsgden)
{
    ASSERT_ERR((((uint32_t)gfskdsgden << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(BESMDM_GFSK_DSG_DEN_ADDR, BESMDM_GFSK_DSG_DEN_MASK, BESMDM_GFSK_DSG_DEN_LSB, gfskdsgden);
}

/**
 * @brief GFSK_DSG_NOM register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  19:10       gfsk_dsg_nom_q   0x0
 *  09:00       gfsk_dsg_nom_i   0x0
 * </pre>
 */
#define BESMDM_GFSK_DSG_NOM_ADDR   0xD0350308
#define BESMDM_GFSK_DSG_NOM_Q_MASK   ((uint32_t)0x000FFC00)
#define BESMDM_GFSK_DSG_NOM_Q_LSB    10
#define BESMDM_GFSK_DSG_NOM_I_MASK   ((uint32_t)0x000003FF)
#define BESMDM_GFSK_DSG_NOM_I_LSB    0
__STATIC_FORCEINLINE void besmdm_gfsk_dsg_nom_pack(uint16_t gfskdsgnomq, uint16_t gfskdsgnomi)
{
    ASSERT_ERR((((uint32_t)gfskdsgnomq << 10) & ~((uint32_t)0x000FFC00)) == 0);
    ASSERT_ERR((((uint32_t)gfskdsgnomi << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(BESMDM_GFSK_DSG_NOM_ADDR, BESMDM_GFSK_DSG_NOM_Q_MASK, BESMDM_GFSK_DSG_NOM_Q_LSB, gfskdsgnomq);
    REG_RPL_WR(BESMDM_GFSK_DSG_NOM_ADDR, BESMDM_GFSK_DSG_NOM_I_MASK, BESMDM_GFSK_DSG_NOM_I_LSB, gfskdsgnomi);
}

__STATIC_FORCEINLINE uint16_t besmdm_gfsk_dsg_nom_i_getf(void)
{
    uint32_t localVal = BTDIGITAL_REG(BESMDM_GFSK_DSG_NOM_ADDR);
    return ((localVal & ((uint32_t)0x000003FF)) >> 0);
}

__STATIC_FORCEINLINE uint16_t besmdm_gfsk_dsg_nom_q_getf(void)
{
    uint32_t localVal = BTDIGITAL_REG(BESMDM_GFSK_DSG_NOM_ADDR);
    return ((localVal & ((uint32_t)0x000FFC00)) >> 10);
}

__STATIC_FORCEINLINE void besmdm_gfsk_dsg_nom_i_setf(uint16_t gfskdsgnomi)
{
    ASSERT_ERR((((uint32_t)gfskdsgnomi << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(BESMDM_GFSK_DSG_NOM_ADDR, BESMDM_GFSK_DSG_NOM_I_MASK, BESMDM_GFSK_DSG_NOM_I_LSB, gfskdsgnomi);
}

__STATIC_FORCEINLINE void besmdm_gfsk_dsg_nom_q_setf(uint16_t gfskdsgnomq)
{
    ASSERT_ERR((((uint32_t)gfskdsgnomq << 10) & ~((uint32_t)0x000FFC00)) == 0);
    REG_RPL_WR(BESMDM_GFSK_DSG_NOM_ADDR, BESMDM_GFSK_DSG_NOM_Q_MASK, BESMDM_GFSK_DSG_NOM_Q_LSB, gfskdsgnomq);
}

/**
 * @brief DPSK_DSG_DEN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  03:00         dpsk_dsg_den   0x0
 * </pre>
 */
#define BESMDM_DPSK_DSG_DEN_ADDR   0xD0350340
#define BESMDM_DPSK_DSG_DEN_MASK   ((uint32_t)0x0000000F)
#define BESMDM_DPSK_DSG_DEN_LSB    0
__STATIC_FORCEINLINE void besmdm_dpsk_dsg_den_setf(uint8_t dpskdsgden)
{
    ASSERT_ERR((((uint32_t)dpskdsgden << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(BESMDM_DPSK_DSG_DEN_ADDR, BESMDM_DPSK_DSG_DEN_MASK, BESMDM_DPSK_DSG_DEN_LSB, dpskdsgden);
}

/**
 * @brief DPSK_DSG_NOM register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  09:00         dpsk_dsg_nom   0x0
 * </pre>
 */
#define BESMDM_DPSK_DSG_NOM_ADDR   0xD0350344
#define BESMDM_DPSK_DSG_NOM_MASK   ((uint32_t)0x000003FF)
#define BESMDM_DPSK_DSG_NOM_LSB    0
__STATIC_FORCEINLINE void besmdm_dpsk_dsg_nom_setf(uint16_t dpskdsgnom)
{
    ASSERT_ERR((((uint32_t)dpskdsgnom << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(BESMDM_DPSK_DSG_NOM_ADDR, BESMDM_DPSK_DSG_NOM_MASK, BESMDM_DPSK_DSG_NOM_LSB, dpskdsgnom);
}

__STATIC_FORCEINLINE uint16_t besmdm_dpsk_dsg_nom_getf(void)
{
    uint32_t localVal = BTDIGITAL_REG(BESMDM_DPSK_DSG_NOM_ADDR);
    ASSERT_ERR((localVal & ~((uint32_t)0x000003FF)) == 0);
    return (localVal >> 0);
}

/**
 * @brief TX_IQ_SWAP_EN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     16        tx_iq_swap_en   0
 * </pre>
 */
#define BESMDM_TX_IQ_SWAP_EN_ADDR   0xD0350214
#define BESMDM_TXI_SIGNED_BIT       ((uint32_t)0x00080000)
#define BESMDM_TXI_SIGNED_POS       19
#define BESMDM_TXQ_SIGNED_BIT       ((uint32_t)0x00040000)
#define BESMDM_TXQ_SIGNED_POS       18
#define BESMDM_TX_IQ_SWAP_EN_BIT    ((uint32_t)0x00010000)
#define BESMDM_TX_IQ_SWAP_EN_POS    16
__STATIC_FORCEINLINE void besmdm_tx_iq_swap_en_pack(uint8_t txisigned, uint8_t txqsigned, uint8_t txiqswapen)
{
    ASSERT_ERR((((uint32_t)txisigned << 19) & ~((uint32_t)0x00080000)) == 0);
    ASSERT_ERR((((uint32_t)txqsigned << 18) & ~((uint32_t)0x00040000)) == 0);
    ASSERT_ERR((((uint32_t)txiqswapen << 16) & ~((uint32_t)0x00010000)) == 0);
    REG_RPL_WR(BESMDM_TX_IQ_SWAP_EN_ADDR, BESMDM_TX_IQ_SWAP_EN_BIT, BESMDM_TX_IQ_SWAP_EN_POS, txisigned);
    REG_RPL_WR(BESMDM_TX_IQ_SWAP_EN_ADDR, BESMDM_TXQ_SIGNED_BIT, BESMDM_TXQ_SIGNED_POS, txqsigned);
    REG_RPL_WR(BESMDM_TX_IQ_SWAP_EN_ADDR, BESMDM_TXI_SIGNED_BIT, BESMDM_TXI_SIGNED_POS, txiqswapen);
}

/**
 * @brief RX_DPSK_NEW_MODE_EN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     00   rx_dpsk_new_mode_en   0
 * </pre>
 */
#define BESMDM_RX_DPSK_NEW_MODE_EN_ADDR   0xD03501E8
#define BESMDM_STO_CNT_LEN_MASK           ((uint32_t)0xFFC00000)
#define BESMDM_STO_CNT_LEN_LSB            22
#define BESMDM_RX_DPSK_NEW_MODE_EN_BIT    ((uint32_t)0x00000001)
#define BESMDM_RX_DPSK_NEW_MODE_EN_POS    0

__STATIC_FORCEINLINE void besmdm_rx_dpsk_new_mode_en_pack(uint16_t stocntlen, uint8_t rxdpsknewmodeen)
{
    ASSERT_ERR((((uint32_t)stocntlen << 22) & ~((uint32_t)0xFFC00000)) == 0);
    ASSERT_ERR((((uint32_t)rxdpsknewmodeen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(BESMDM_RX_DPSK_NEW_MODE_EN_ADDR, BESMDM_STO_CNT_LEN_MASK, BESMDM_STO_CNT_LEN_LSB, stocntlen);
    REG_RPL_WR(BESMDM_RX_DPSK_NEW_MODE_EN_ADDR, BESMDM_RX_DPSK_NEW_MODE_EN_BIT, BESMDM_RX_DPSK_NEW_MODE_EN_POS, rxdpsknewmodeen);
}


/**
 * @brief EDR_GFSK_DSG_NOM register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     23      edr_gfsk_dsg_en   0
 *  09:00          gsg_edr_nom   0x0
 * </pre>
 */
#define BESMDM_EDR_GFSK_DSG_NOM_ADDR   0xD0350370
#define BESMDM_EDR_GFSK_DSG_EN_BIT    ((uint32_t)0x00800000)
#define BESMDM_EDR_GFSK_DSG_EN_POS    23
#define BESMDM_GSG_EDR_NOM_MASK       ((uint32_t)0x000003FF)
#define BESMDM_GSG_EDR_NOM_LSB        0
__STATIC_FORCEINLINE void besmdm_edr_gfsk_dsg_nom_pack(uint8_t edrgfskdsgen, uint16_t gsgedrnom)
{
    ASSERT_ERR((((uint32_t)edrgfskdsgen << 23) & ~((uint32_t)0x00800000)) == 0);
    ASSERT_ERR((((uint32_t)gsgedrnom << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(BESMDM_EDR_GFSK_DSG_NOM_ADDR, BESMDM_EDR_GFSK_DSG_EN_BIT, BESMDM_EDR_GFSK_DSG_EN_POS, edrgfskdsgen);
    REG_RPL_WR(BESMDM_EDR_GFSK_DSG_NOM_ADDR, BESMDM_GSG_EDR_NOM_MASK, BESMDM_GSG_EDR_NOM_LSB, gsgedrnom);
}

__STATIC_FORCEINLINE uint16_t besmdm_gsg_edr_nom_getf(void)
{
    uint32_t localVal = BTDIGITAL_REG(BESMDM_EDR_GFSK_DSG_NOM_ADDR);
    return ((localVal & ((uint32_t)0x000003FF)) >> 0);
}

__STATIC_FORCEINLINE void besmdm_gsg_edr_nom_setf(uint16_t gsgedrnom)
{
    ASSERT_ERR((((uint32_t)gsgedrnom << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(BESMDM_EDR_GFSK_DSG_NOM_ADDR, BESMDM_GSG_EDR_NOM_MASK, BESMDM_GSG_EDR_NOM_LSB, gsgedrnom);
}

/**
 * @brief TX_STARTUPDEL register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  07:00        tx_startupdel   0x0
 * </pre>
 */
#define BESMDM_TX_STARTUPDEL_ADDR   0xD0350210
#define BESMDM_TX_STARTUPDEL_MASK   ((uint32_t)0x000000FF)
#define BESMDM_TX_STARTUPDEL_LSB    0
__STATIC_FORCEINLINE void besmdm_tx_startupdel_setf(uint8_t txstartupdel)
{
    ASSERT_ERR((((uint32_t)txstartupdel << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(BESMDM_TX_STARTUPDEL_ADDR, BESMDM_TX_STARTUPDEL_MASK, BESMDM_TX_STARTUPDEL_LSB, txstartupdel);
}

/**
 * @brief RAMP_MODE_DN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  04:03         ramp_mode_dn   0x0
 * </pre>
 */
#define BESMDM_RAMP_MODE_DN_ADDR   0xD0350254
#define BESMDM_RAMP_MODE_DN_MASK   ((uint32_t)0x00000018)
#define BESMDM_RAMP_MODE_DN_LSB    3
__STATIC_FORCEINLINE void besmdm_ramp_mode_dn_setf(uint8_t rampmodedn)
{
    ASSERT_ERR((((uint32_t)rampmodedn << 4) & ~((uint32_t)0x00000030)) == 0);
    REG_RPL_WR(BESMDM_RAMP_MODE_DN_ADDR, BESMDM_RAMP_MODE_DN_MASK, BESMDM_RAMP_MODE_DN_LSB, rampmodedn);
}

/**
 * @brief AHI_HTX_TRACKING_1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  07:04     ahi_k1_parameter   0x0
 * </pre>
 */
#define BESMDM_AHI_HTX_TRACKING_1_ADDR   0xD0350004
// field definitions
#define BESMDM_AHI_K_1_PARAMETER_MASK   ((uint32_t)0x000000F0)
#define BESMDM_AHI_K_1_PARAMETER_LSB    4
__STATIC_FORCEINLINE void besmdm_ahi_k_1_parameter_setf(uint8_t ahik1parameter)
{
    ASSERT_ERR((((uint32_t)ahik1parameter << 4) & ~((uint32_t)0x000000F0)) == 0);
    REG_RPL_WR(BESMDM_AHI_HTX_TRACKING_1_ADDR, BESMDM_AHI_K_1_PARAMETER_MASK, BESMDM_AHI_K_1_PARAMETER_LSB, ahik1parameter);
}

/**
 * @brief AHI_HTX_TRACKING_2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  29:16        ahi_rep_cntth   0x0
 *  03:00           ahi_mean_k   0x0
 * </pre>
 */
#define BESMDM_AHI_HTX_TRACKING_2_ADDR   0xD0350374
#define BESMDM_AHI_REP_CNTTH_MASK   ((uint32_t)0x3FFF0000)
#define BESMDM_AHI_REP_CNTTH_LSB    16
#define BESMDM_AHI_MEAN_K_MASK      ((uint32_t)0x0000000F)
#define BESMDM_AHI_MEAN_K_LSB       0
__STATIC_FORCEINLINE void besmdm_ahi_htx_tracking_2_pack(uint16_t ahirepcntth, uint8_t ahimeank)
{
    ASSERT_ERR((((uint32_t)ahirepcntth << 16) & ~((uint32_t)0x3FFF0000)) == 0);
    ASSERT_ERR((((uint32_t)ahimeank << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(BESMDM_AHI_HTX_TRACKING_2_ADDR, BESMDM_AHI_REP_CNTTH_MASK, BESMDM_AHI_REP_CNTTH_LSB, ahirepcntth);
    REG_RPL_WR(BESMDM_AHI_HTX_TRACKING_2_ADDR, BESMDM_AHI_MEAN_K_MASK, BESMDM_AHI_MEAN_K_LSB, ahimeank);
}

/**
 * @brief AHI_HTX_TRACKING_3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  29:16          ahi_htx_thl   0x0
 *  11:00          ahi_htx_thh   0x0
 * </pre>
 */
#define BESMDM_AHI_HTX_TRACKING_3_ADDR   0xD0350378
#define BESMDM_AHI_HTX_THL_MASK   ((uint32_t)0x3FFF0000)
#define BESMDM_AHI_HTX_THL_LSB    16
#define BESMDM_AHI_HTX_THH_MASK   ((uint32_t)0x00000FFF)
#define BESMDM_AHI_HTX_THH_LSB    0
__STATIC_FORCEINLINE void besmdm_ahi_htx_tracking_3_pack(uint16_t ahihtxthl, uint16_t ahihtxthh)
{
    ASSERT_ERR((((uint32_t)ahihtxthl << 16) & ~((uint32_t)0x3FFF0000)) == 0);
    ASSERT_ERR((((uint32_t)ahihtxthh << 0) & ~((uint32_t)0x00000FFF)) == 0);
    REG_RPL_WR(BESMDM_AHI_HTX_TRACKING_3_ADDR,  BESMDM_AHI_HTX_THL_MASK, BESMDM_AHI_HTX_THL_LSB, ahihtxthl);
    REG_RPL_WR(BESMDM_AHI_HTX_TRACKING_3_ADDR,  BESMDM_AHI_HTX_THH_MASK, BESMDM_AHI_HTX_THH_LSB, ahihtxthh);
}

/**
 * @brief BT_PSD_FILTER_ON_1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     03   psd_filterbypass_bt   0
 * </pre>
 */
#define BESMDM_BT_PSD_FILTER_ON_1_ADDR   0xD03500C0
#define BESMDM_PSD_FILTERBYPASS_BT_BIT    ((uint32_t)0x00000008)
#define BESMDM_PSD_FILTERBYPASS_BT_POS    3
__STATIC_FORCEINLINE void besmdm_psd_filterbypass_bt_setf(uint8_t psdfilterbypassbt)
{
    ASSERT_ERR((((uint32_t)psdfilterbypassbt << 3) & ~((uint32_t)0x00000008)) == 0);
    REG_RPL_WR(BESMDM_BT_PSD_FILTER_ON_1_ADDR, BESMDM_PSD_FILTERBYPASS_BT_BIT, BESMDM_PSD_FILTERBYPASS_BT_POS, psdfilterbypassbt);
}

/**
 * @brief BT_PSD_FILTER_ON_2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  26:16     bt_timeinit_bw4m   0x0
 *  10:00     te_timeinit_bw2m   0x0
 * </pre>
 */
#define BESMDM_BT_PSD_FILTER_ON_2_ADDR   0xD03500D4
#define BESMDM_BT_TIMEINIT_BW_4M_MASK   ((uint32_t)0x07FF0000)
#define BESMDM_BT_TIMEINIT_BW_4M_LSB    16
#define BESMDM_TE_TIMEINIT_BW_2M_MASK   ((uint32_t)0x000007FF)
#define BESMDM_TE_TIMEINIT_BW_2M_LSB    0
__STATIC_FORCEINLINE void besmdm_bt_psd_filter_on_2_pack(uint16_t bttimeinitbw4m, uint16_t tetimeinitbw2m)
{
    ASSERT_ERR((((uint32_t)bttimeinitbw4m << 16) & ~((uint32_t)0x07FF0000)) == 0);
    ASSERT_ERR((((uint32_t)tetimeinitbw2m << 0) & ~((uint32_t)0x000007FF)) == 0);
    REG_RPL_WR(BESMDM_BT_PSD_FILTER_ON_2_ADDR, BESMDM_BT_TIMEINIT_BW_4M_MASK, BESMDM_BT_TIMEINIT_BW_4M_LSB, bttimeinitbw4m);
    REG_RPL_WR(BESMDM_BT_PSD_FILTER_ON_2_ADDR, BESMDM_TE_TIMEINIT_BW_2M_MASK, BESMDM_TE_TIMEINIT_BW_2M_LSB, tetimeinitbw2m);
}

/**
 * @brief BT_PSD_FILTER_ON_3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:21       te_timeinit_fa   0x0
 *  10:00    be_timeinit_bw_1M   0x0
 * </pre>
 */
#define BESMDM_BT_PSD_FILTER_ON_3_ADDR   0xD03502C4
#define BESMDM_TE_TIMEINIT_FA_MASK       ((uint32_t)0xFFE00000)
#define BESMDM_TE_TIMEINIT_FA_LSB        21
#define BESMDM_BE_TIMEINIT_BW__1M_MASK   ((uint32_t)0x000007FF)
#define BESMDM_BE_TIMEINIT_BW__1M_LSB    0
__STATIC_FORCEINLINE void besmdm_bt_psd_filter_on_3_pack(uint16_t tetimeinitfa, uint16_t betimeinitbw1m)
{
    ASSERT_ERR((((uint32_t)tetimeinitfa << 21) & ~((uint32_t)0xFFE00000)) == 0);
    ASSERT_ERR((((uint32_t)betimeinitbw1m << 0) & ~((uint32_t)0x000007FF)) == 0);
    REG_RPL_WR(BESMDM_BT_PSD_FILTER_ON_3_ADDR, BESMDM_TE_TIMEINIT_FA_MASK, BESMDM_TE_TIMEINIT_FA_LSB, tetimeinitfa);
    REG_RPL_WR(BESMDM_BT_PSD_FILTER_ON_3_ADDR, BESMDM_BE_TIMEINIT_BW__1M_MASK, BESMDM_BE_TIMEINIT_BW__1M_LSB, betimeinitbw1m);
}

/**
 * @brief RX_PWR_EST register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  02:00    rxpwr_k_parameter   0x0
 * </pre>
 */
#define BESMDM_RX_PWR_EST_ADDR   0xD0350018
#define BESMDM_RXPWR_K_PARAMETER_MASK   ((uint32_t)0x00000007)
#define BESMDM_RXPWR_K_PARAMETER_LSB    0
__STATIC_FORCEINLINE void besmdm_rxpwr_k_parameter_setf(uint8_t rxpwrkparameter)
{
    ASSERT_ERR((((uint32_t)rxpwrkparameter << 0) & ~((uint32_t)0x00000007)) == 0);
    REG_RPL_WR(BESMDM_RX_PWR_EST_ADDR, BESMDM_RXPWR_K_PARAMETER_MASK, BESMDM_RXPWR_K_PARAMETER_LSB, rxpwrkparameter);
}

/**
 * @brief SYNC_PARAMETER_1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  20:18     synb_adj_bt_bw4m   0x0
 *  17:15    synb_adj_bt_bw_2m   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_1_ADDR   0xD03501BC
#define BESMDM_SYNB_ADJ_BT_BW_4M_MASK    ((uint32_t)0x001C0000)
#define BESMDM_SYNB_ADJ_BT_BW_4M_LSB     18
#define BESMDM_SYNB_ADJ_BT_BW_2M_MASK   ((uint32_t)0x00038000)
#define BESMDM_SYNB_ADJ_BT_BW_2M_LSB    15
__STATIC_FORCEINLINE void besmdm_sync_parameter_1_pack(uint8_t synbadjbtbw4m, uint8_t synbadjbtbw2m)
{
    ASSERT_ERR((((uint32_t)synbadjbtbw4m << 18) & ~((uint32_t)0x001C0000)) == 0);
    ASSERT_ERR((((uint32_t)synbadjbtbw2m << 15) & ~((uint32_t)0x00038000)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_1_ADDR,  BESMDM_SYNB_ADJ_BT_BW_4M_MASK, BESMDM_SYNB_ADJ_BT_BW_4M_LSB, synbadjbtbw4m);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_1_ADDR,  BESMDM_SYNB_ADJ_BT_BW_2M_MASK, BESMDM_SYNB_ADJ_BT_BW_2M_LSB, synbadjbtbw2m);
}

/**
 * @brief SYNC_PARAMETER_2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:29          sync_adj_bt   0x0
 *  23:20        max_err_th_bt   0x0
 *  08:00            par_th_bt   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_2_ADDR   0xD03501C0
#define BESMDM_SYNC_ADJ_BT_MASK     ((uint32_t)0xE0000000)
#define BESMDM_SYNC_ADJ_BT_LSB      29
#define BESMDM_MAX_ERR_TH_BT_MASK   ((uint32_t)0x00F00000)
#define BESMDM_MAX_ERR_TH_BT_LSB    20
#define BESMDM_PAR_TH_BT_MASK       ((uint32_t)0x000001FF)
#define BESMDM_PAR_TH_BT_LSB        0
__STATIC_FORCEINLINE void besmdm_sync_parameter_2_pack(uint8_t syncadjbt, uint8_t maxerrthbt, uint16_t parthbt)
{
    ASSERT_ERR((((uint32_t)maxerrthbt << 20) & ~((uint32_t)0x00F00000)) == 0);
    ASSERT_ERR((((uint32_t)parthbt << 0) & ~((uint32_t)0x000001FF)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_2_ADDR,  BESMDM_SYNC_ADJ_BT_MASK, BESMDM_SYNC_ADJ_BT_LSB, syncadjbt);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_2_ADDR,  BESMDM_MAX_ERR_TH_BT_MASK, BESMDM_MAX_ERR_TH_BT_LSB, maxerrthbt);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_2_ADDR,  BESMDM_PAR_TH_BT_MASK, BESMDM_PAR_TH_BT_LSB, parthbt);
}

/**
 * @brief SYNC_PARAMETER_3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:28     max_err_th_ble2m   0x0
 *  27:24      max_err_the_ble   0x0
 *  23:15           par_th_ble   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_3_ADDR   0xD03501C4
#define BESMDM_MAX_ERR_TH_BLE_2M_MASK   ((uint32_t)0xF0000000)
#define BESMDM_MAX_ERR_TH_BLE_2M_LSB    28
#define BESMDM_MAX_ERR_THE_BLE_MASK     ((uint32_t)0x0F000000)
#define BESMDM_MAX_ERR_THE_BLE_LSB      24
#define BESMDM_PAR_TH_BLE_MASK          ((uint32_t)0x00FF8000)
#define BESMDM_PAR_TH_BLE_LSB           15
__STATIC_FORCEINLINE void besmdm_sync_parameter_3_pack(uint8_t maxerrthble2m, uint8_t maxerrtheble, uint16_t parthble)
{
    ASSERT_ERR((((uint32_t)maxerrthble2m << 28) & ~((uint32_t)0xF0000000)) == 0);
    ASSERT_ERR((((uint32_t)maxerrtheble << 24) & ~((uint32_t)0x0F000000)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_3_ADDR, BESMDM_MAX_ERR_TH_BLE_2M_MASK, BESMDM_MAX_ERR_TH_BLE_2M_LSB, maxerrthble2m);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_3_ADDR, BESMDM_MAX_ERR_THE_BLE_MASK, BESMDM_MAX_ERR_THE_BLE_LSB, maxerrtheble);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_3_ADDR, BESMDM_PAR_TH_BLE_MASK, BESMDM_PAR_TH_BLE_LSB, parthble);
}

/**
 * @brief SYNC_PARAMETER_4 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:28     max_err_th_ble4m   0x0
 *  27:24        max_err_th_fa   0x0
 *  23:15            par_th_fa   0x0
 *  14:00       spwr_min_th_fa   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_4_ADDR   0xD03501C8
#define BESMDM_MAX_ERR_TH_BLE_4M_MASK   ((uint32_t)0xF0000000)
#define BESMDM_MAX_ERR_TH_BLE_4M_LSB    28
#define BESMDM_MAX_ERR_TH_FA_MASK       ((uint32_t)0x0F000000)
#define BESMDM_MAX_ERR_TH_FA_LSB        24
#define BESMDM_PAR_TH_FA_MASK           ((uint32_t)0x00FF8000)
#define BESMDM_PAR_TH_FA_LSB            15
#define BESMDM_SPWR_MIN_TH_FA_MASK      ((uint32_t)0x00007FFF)
#define BESMDM_SPWR_MIN_TH_FA_LSB       0
__STATIC_FORCEINLINE void besmdm_sync_parameter_4_pack(uint8_t maxerrthble4m, uint8_t maxerrthfa, uint16_t parthfa, uint16_t spwrminthfa)
{
    ASSERT_ERR((((uint32_t)maxerrthble4m << 28) & ~((uint32_t)0xF0000000)) == 0);
    ASSERT_ERR((((uint32_t)maxerrthfa << 24) & ~((uint32_t)0x0F000000)) == 0);
    ASSERT_ERR((((uint32_t)parthfa << 15) & ~((uint32_t)0x00FF8000)) == 0);
    ASSERT_ERR((((uint32_t)spwrminthfa << 0) & ~((uint32_t)0x00007FFF)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_4_ADDR, BESMDM_MAX_ERR_TH_BLE_4M_MASK, BESMDM_MAX_ERR_TH_BLE_4M_LSB, maxerrthble4m);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_4_ADDR, BESMDM_MAX_ERR_TH_FA_MASK, BESMDM_MAX_ERR_TH_FA_LSB, maxerrthfa);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_4_ADDR, BESMDM_PAR_TH_FA_MASK, BESMDM_PAR_TH_FA_LSB, parthfa);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_4_ADDR, BESMDM_SPWR_MIN_TH_FA_MASK, BESMDM_SPWR_MIN_TH_FA_LSB, spwrminthfa);
}

/**
 * @brief SYNC_PARAMETER_5 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  28:20         par_th_ble2m   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_5_ADDR   0xD03501CC
#define BESMDM_PAR_TH_BLE_2M_MASK   ((uint32_t)0x1FF00000)
#define BESMDM_PAR_TH_BLE_2M_LSB    20
__STATIC_FORCEINLINE void besmdm_par_th_ble_2m_setf(uint16_t parthble2m)
{
    ASSERT_ERR((((uint32_t)parthble2m << 20) & ~((uint32_t)0x1FF00000)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_5_ADDR, BESMDM_PAR_TH_BLE_2M_MASK, BESMDM_PAR_TH_BLE_2M_LSB, parthble2m);
}

/**
 * @brief SYNC_PARAMETER_6 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  25:22   max_err_th_bt_bw4m   0x0
 *  21:18   max_err_th_bt_bw2m   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_6_ADDR   0xD03501D0
#define BESMDM_MAX_ERR_TH_BT_BW_4M_MASK   ((uint32_t)0x03C00000)
#define BESMDM_MAX_ERR_TH_BT_BW_4M_LSB    22
#define BESMDM_MAX_ERR_TH_BT_BW_2M_MASK   ((uint32_t)0x003C0000)
#define BESMDM_MAX_ERR_TH_BT_BW_2M_LSB    18
__STATIC_FORCEINLINE void besmdm_sync_parameter_6_pack(uint8_t maxerrthbtbw4m, uint8_t maxerrthbtbw2m)
{
    ASSERT_ERR((((uint32_t)maxerrthbtbw4m << 22) & ~((uint32_t)0x03C00000)) == 0);
    ASSERT_ERR((((uint32_t)maxerrthbtbw2m << 18) & ~((uint32_t)0x003C0000)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_6_ADDR, BESMDM_MAX_ERR_TH_BT_BW_4M_MASK, BESMDM_MAX_ERR_TH_BT_BW_4M_LSB, maxerrthbtbw4m);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_6_ADDR, BESMDM_MAX_ERR_TH_BT_BW_2M_MASK, BESMDM_MAX_ERR_TH_BT_BW_2M_LSB, maxerrthbtbw2m);
}

/**
 * @brief SYNC_PARAMETER_7 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00            par_th_bt   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_7_ADDR   0xD03501B4
#define BESMDM_PAR_TH_BT_MASK_1   ((uint32_t)0xFFFFFFFF)
#define BESMDM_PAR_TH_BT_LSB_1    0
__STATIC_FORCEINLINE void besmdm_par_th_bt_setf(uint32_t parthbt)
{
    ASSERT_ERR((((uint32_t)parthbt << 0) & ~((uint32_t)0xFFFFFFFF)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_7_ADDR, BESMDM_PAR_TH_BT_MASK_1, BESMDM_PAR_TH_BT_LSB_1, parthbt);
}

/**
 * @brief SYNC_PARAMETER_8 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:00       err_sum_max_th   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_8_ADDR   0xD0350414
#define BESMDM_ERR_SUM_MAX_TH_MASK   ((uint32_t)0x00FFFFFF)
#define BESMDM_ERR_SUM_MAX_TH_LSB    0
__STATIC_FORCEINLINE void besmdm_err_sum_max_th_setf(uint32_t errsummaxth)
{
    ASSERT_ERR((((uint32_t)errsummaxth << 0) & ~((uint32_t)0x00FFFFFF)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_8_ADDR, BESMDM_ERR_SUM_MAX_TH_MASK, BESMDM_ERR_SUM_MAX_TH_LSB, errsummaxth);
}

/**
 * @brief SYNC_PARAMETER_9 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:16            rx_pwr_th   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_9_ADDR   0xD0350230
#define BESMDM_RX_PWR_TH_MASK   ((uint32_t)0xFFFF0000)
#define BESMDM_RX_PWR_TH_LSB    16
__STATIC_FORCEINLINE void besmdm_rx_pwr_th_setf(uint16_t rxpwrth)
{
    ASSERT_ERR((((uint32_t)rxpwrth << 16) & ~((uint32_t)0xFFFF0000)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_9_ADDR, BESMDM_RX_PWR_TH_MASK, BESMDM_RX_PWR_TH_LSB, rxpwrth);
}

/**
 * @brief SYNC_PARAMETER_10 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:16        rxpwr_th_bw2m   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_10_ADDR   0xD0350234
#define BESMDM_RXPWR_TH_BW_2M_MASK   ((uint32_t)0xFFFF0000)
#define BESMDM_RXPWR_TH_BW_2M_LSB    16
__STATIC_FORCEINLINE void besmdm_rxpwr_th_bw_2m_setf(uint16_t rxpwrthbw2m)
{
    ASSERT_ERR((((uint32_t)rxpwrthbw2m << 16) & ~((uint32_t)0xFFFF0000)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_10_ADDR, BESMDM_RXPWR_TH_BW_2M_MASK, BESMDM_RXPWR_TH_BW_2M_LSB, rxpwrthbw2m);
}

/**
 * @brief SYNC_PARAMETER_11 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:16        rxpwr_th_bw4m   0x0
 * </pre>
 */
#define BESMDM_SYNC_PARAMETER_11_ADDR   0xD0350238
#define BESMDM_RXPWR_TH_BW_4M_MASK   ((uint32_t)0xFFFF0000)
#define BESMDM_RXPWR_TH_BW_4M_LSB    16
__STATIC_FORCEINLINE void besmdm_rxpwr_th_bw_4m_setf(uint16_t rxpwrthbw4m)
{
    ASSERT_ERR((((uint32_t)rxpwrthbw4m << 16) & ~((uint32_t)0xFFFF0000)) == 0);
    REG_RPL_WR(BESMDM_SYNC_PARAMETER_11_ADDR, BESMDM_RXPWR_TH_BW_4M_MASK, BESMDM_RXPWR_TH_BW_4M_LSB, rxpwrthbw4m);
}

/**
 * @brief RC_IN_FILTER_ENABLE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  03:01      rx_in_filter_en   0x0
 * </pre>
 */
#define BESMDM_RC_IN_FILTER_ENABLE_ADDR   0xD035025C
#define BESMDM_RX_IN_FILTER_EN_MASK   ((uint32_t)0x0000000E)
#define BESMDM_RX_IN_FILTER_EN_LSB    1
__STATIC_FORCEINLINE void besmdm_rx_in_filter_en_setf(uint8_t rxinfilteren)
{
    ASSERT_ERR((((uint32_t)rxinfilteren << 1) & ~((uint32_t)0x0000000E)) == 0);
    REG_RPL_WR(BESMDM_RC_IN_FILTER_ENABLE_ADDR, BESMDM_RX_IN_FILTER_EN_MASK, BESMDM_RX_IN_FILTER_EN_LSB, rxinfilteren);
}

/**
 * @brief EDR_SKY_ON register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  15:08        dpsk_k3_edr3m   0x0
 * </pre>
 */
#define BESMDM_EDR_SKY_ON_ADDR   0xD03501F0
#define BESMDM_DPSK_K_3_EDR_3M_MASK   ((uint32_t)0x0000FF00)
#define BESMDM_DPSK_K_3_EDR_3M_LSB    8
__STATIC_FORCEINLINE void besmdm_dpsk_k_3_edr_3m_setf(uint8_t dpskk3edr3m)
{
    ASSERT_ERR((((uint32_t)dpskk3edr3m << 8) & ~((uint32_t)0x0000FF00)) == 0);
    REG_RPL_WR(BESMDM_EDR_SKY_ON_ADDR, BESMDM_DPSK_K_3_EDR_3M_MASK, BESMDM_DPSK_K_3_EDR_3M_LSB, dpskk3edr3m);
}

/**
 * @brief IQMIS_LMS_ON_1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     00        iqmis_comp_en   0
 * </pre>
 */
#define BESMDM_IQMIS_LMS_ON_1_ADDR   0xD0350040
#define BESMDM_IQMIS_COMP_EN_BIT    ((uint32_t)0x00000001)
#define BESMDM_IQMIS_COMP_EN_POS    0
__STATIC_FORCEINLINE void besmdm_iqmis_comp_en_setf(uint8_t iqmiscompen)
{
    ASSERT_ERR((((uint32_t)iqmiscompen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(BESMDM_IQMIS_LMS_ON_1_ADDR, BESMDM_IQMIS_COMP_EN_BIT, BESMDM_IQMIS_COMP_EN_POS, iqmiscompen);
}

/**
 * @brief IQMIS_LMS_ON_2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  19:17        iqmis_comp_en   0x0
 * </pre>
 */
#define BESMDM_IQMIS_LMS_ON_2_ADDR   0xD035005C
#define BESMDM_IQMIS_COMP_EN_MASK   ((uint32_t)0x000E0000)
#define BESMDM_IQMIS_COMP_EN_LSB    17
__STATIC_FORCEINLINE void besmdm_iqmis_comp_setf(uint8_t iqmiscompen)
{
    ASSERT_ERR((((uint32_t)iqmiscompen << 17) & ~((uint32_t)0x000E0000)) == 0);
    REG_RPL_WR(BESMDM_IQMIS_LMS_ON_2_ADDR, BESMDM_IQMIS_COMP_EN_MASK, BESMDM_IQMIS_COMP_EN_LSB, iqmiscompen);
}

/**
 * @brief IQMIS_LMS_ON_3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:16     iqmis_time_th_lr   0x0
 *  15:00        iqmis_time_th   0x0
 * </pre>
 */
#define BESMDM_IQMIS_LMS_ON_3_ADDR   0xD0350120
#define BESMDM_IQMIS_TIME_TH_LR_MASK   ((uint32_t)0xFFFF0000)
#define BESMDM_IQMIS_TIME_TH_LR_LSB    16
#define BESMDM_IQMIS_TIME_TH_MASK      ((uint32_t)0x0000FFFF)
#define BESMDM_IQMIS_TIME_TH_LSB       0
__STATIC_FORCEINLINE void besmdm_iqmis_lms_on_3_pack(uint16_t iqmistimethlr, uint16_t iqmistimeth)
{
    ASSERT_ERR((((uint32_t)iqmistimethlr << 16) & ~((uint32_t)0xFFFF0000)) == 0);
    ASSERT_ERR((((uint32_t)iqmistimeth << 0) & ~((uint32_t)0x0000FFFF)) == 0);
    REG_RPL_WR(BESMDM_IQMIS_LMS_ON_3_ADDR, BESMDM_IQMIS_TIME_TH_LR_MASK, BESMDM_IQMIS_TIME_TH_LR_LSB, iqmistimethlr);
    REG_RPL_WR(BESMDM_IQMIS_LMS_ON_3_ADDR, BESMDM_IQMIS_TIME_TH_MASK, BESMDM_IQMIS_TIME_TH_LSB, iqmistimeth);
}

/**
 * @brief VALID_POSITION register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  06:04    osr_12_valib_posi   0x0
 * </pre>
 */
#define BESMDM_VALID_POSITION_ADDR   0xD0350440
#define BESMDM_OSR__12_VALIB_POSI_MASK   ((uint32_t)0x00000070)
#define BESMDM_OSR__12_VALIB_POSI_LSB    4
__STATIC_FORCEINLINE void besmdm_osr__12_valib_posi_setf(uint8_t osr12valibposi)
{
    ASSERT_ERR((((uint32_t)osr12valibposi << 4) & ~((uint32_t)0x00000070)) == 0);
    REG_RPL_WR(BESMDM_VALID_POSITION_ADDR, BESMDM_OSR__12_VALIB_POSI_MASK, BESMDM_OSR__12_VALIB_POSI_LSB, osr12valibposi);
}

/** @brief HW_AGC_PWR_LOCK register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     04     pwr_lock_en_slow   0
 *     03          pwr_lock_en   0
 * </pre>
 */
#define BESMDM_HW_AGC_PWR_LOCK_ADDR    0xD03500E0
#define BESMDM_PWR_LOCK_EN_SLOW_BIT    ((uint32_t)0x00000010)
#define BESMDM_PWR_LOCK_EN_SLOW_POS    4
#define BESMDM_PWR_LOCK_EN_BIT         ((uint32_t)0x00000008)
#define BESMDM_PWR_LOCK_EN_POS         3
__STATIC_FORCEINLINE void besmdm_hw_agc_pwr_lock_pack(uint8_t pwrlockenslow, uint8_t pwrlocken)
{
    ASSERT_ERR((((uint32_t)pwrlockenslow << 4) & ~((uint32_t)0x00000010)) == 0);
    ASSERT_ERR((((uint32_t)pwrlocken << 3) & ~((uint32_t)0x00000008)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_PWR_LOCK_ADDR, BESMDM_PWR_LOCK_EN_SLOW_BIT, BESMDM_PWR_LOCK_EN_SLOW_POS, pwrlockenslow);
    REG_RPL_WR(BESMDM_HW_AGC_PWR_LOCK_ADDR, BESMDM_PWR_LOCK_EN_BIT, BESMDM_PWR_LOCK_EN_POS, pwrlocken);
}

/**
 * @brief HW_AGC_OMEGA_LOCK_BT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:12     omega_lock_th_bt   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_OMEGA_LOCK_BT_ADDR   0xD03500E4
#define BESMDM_OMEGA_LOCK_TH_BT_MASK       ((uint32_t)0x00FFF000)
#define BESMDM_OMEGA_LOCK_TH_BT_LSB        12
__STATIC_FORCEINLINE void besmdm_omega_lock_th_bt_setf(uint16_t omegalockthbt)
{
    ASSERT_ERR((((uint32_t)omegalockthbt << 12) & ~((uint32_t)0x00FFF000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_OMEGA_LOCK_BT_ADDR, BESMDM_OMEGA_LOCK_TH_BT_MASK, BESMDM_OMEGA_LOCK_TH_BT_LSB, omegalockthbt);
}

/**
 * @brief HW_AGC_OMEGA_LOCK_BLE1M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:12   omega_lock_th_ble1m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_OMEGA_LOCK_BLE1M_ADDR   0xD03500EC
#define BESMDM_OMEGA_LOCK_TH_BLE_1M_MASK      ((uint32_t)0x00FFF000)
#define BESMDM_OMEGA_LOCK_TH_BLE_1M_LSB       12
__STATIC_FORCEINLINE void besmdm_omega_lock_th_ble_1m_setf(uint16_t omegalockthble1m)
{
    ASSERT_ERR((((uint32_t)omegalockthble1m << 12) & ~((uint32_t)0x00FFF000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_OMEGA_LOCK_BLE1M_ADDR, BESMDM_OMEGA_LOCK_TH_BLE_1M_MASK, BESMDM_OMEGA_LOCK_TH_BLE_1M_LSB, omegalockthble1m);
}

/**
 * @brief HW_AGC_OMEGA_LOCK_BLE2M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:12   omega_lock_th_ble2m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_OMEGA_LOCK_BLE2M_ADDR   0xD03500F4
#define BESMDM_OMEGA_LOCK_TH_BLE_2M_MASK      ((uint32_t)0x00FFF000)
#define BESMDM_OMEGA_LOCK_TH_BLE_2M_LSB       12
__STATIC_FORCEINLINE void besmdm_omega_lock_th_ble_2m_setf(uint16_t omegalockthble2m)
{
    ASSERT_ERR((((uint32_t)omegalockthble2m << 12) & ~((uint32_t)0x00FFF000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_OMEGA_LOCK_BLE2M_ADDR, BESMDM_OMEGA_LOCK_TH_BLE_2M_MASK, BESMDM_OMEGA_LOCK_TH_BLE_2M_LSB, omegalockthble2m);
}

/**
 * @brief HW_AGC_K_BLE2M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24              k_ble2m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_K_BLE2M_ADDR   0xD03500F8
#define BESMDM_K_BLE_2M_MASK         ((uint32_t)0xFF000000)
#define BESMDM_K_BLE_2M_LSB          24
__STATIC_FORCEINLINE void besmdm_k_ble_2m_setf(uint8_t kble2m)
{
    ASSERT_ERR((((uint32_t)kble2m << 24) & ~((uint32_t)0xFF000000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_K_BLE2M_ADDR, BESMDM_K_BLE_2M_MASK, BESMDM_K_BLE_2M_LSB, kble2m);
}

/**
 * @brief HW_AGC_OMEGA_LOCK_BT_BW2M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:12   omega_lock_th_bt_bw2m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_OMEGA_LOCK_BT_BW2M_ADDR   0xD0350160
#define BESMDM_OMEGA_LOCK_TH_BT_BW_2M_MASK      ((uint32_t)0x00FFF000)
#define BESMDM_OMEGA_LOCK_TH_BT_BW_2M_LSB       12
__STATIC_FORCEINLINE void besmdm_omega_lock_th_bt_bw_2m_setf(uint16_t omegalockthbtbw2m)
{
    ASSERT_ERR((((uint32_t)omegalockthbtbw2m << 12) & ~((uint32_t)0x00FFF000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_OMEGA_LOCK_BT_BW2M_ADDR, BESMDM_OMEGA_LOCK_TH_BT_BW_2M_MASK, BESMDM_OMEGA_LOCK_TH_BT_BW_2M_LSB, omegalockthbtbw2m);
}

/**
 * @brief HW_AGC_OMEGA_LOCK_BW4M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:12   omega_lock_th_bt_bw4m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_OMEGA_LOCK_BW4M_ADDR   0xD0350168
#define BESMDM_OMEGA_LOCK_TH_BT_BW_4M_MASK   ((uint32_t)0x00FFF000)
#define BESMDM_OMEGA_LOCK_TH_BT_BW_4M_LSB    12
__STATIC_FORCEINLINE void besmdm_omega_lock_th_bt_bw_4m_setf(uint16_t omegalockthbtbw4m)
{
    ASSERT_ERR((((uint32_t)omegalockthbtbw4m << 12) & ~((uint32_t)0x00FFF000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_OMEGA_LOCK_BW4M_ADDR, BESMDM_OMEGA_LOCK_TH_BT_BW_4M_MASK, BESMDM_OMEGA_LOCK_TH_BT_BW_4M_LSB, omegalockthbtbw4m);
}

/**
 * @brief HW_AGC_OMEGA_LOCK_BLE4M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  23:12   omega_lock_th_ble4m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_OMEGA_LOCK_BLE4M_ADDR   0xD0350178
#define BESMDM_OMEGA_LOCK_TH_BLE_4M_MASK      ((uint32_t)0x00FFF000)
#define BESMDM_OMEGA_LOCK_TH_BLE_4M_LSB       12
__STATIC_FORCEINLINE void besmdm_omega_lock_th_ble_4m_setf(uint16_t omegalockthble4m)
{
    ASSERT_ERR((((uint32_t)omegalockthble4m << 12) & ~((uint32_t)0x00FFF000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_OMEGA_LOCK_BLE4M_ADDR, BESMDM_OMEGA_LOCK_TH_BLE_4M_MASK, BESMDM_OMEGA_LOCK_TH_BLE_4M_LSB, omegalockthble4m);
}

/**
 * @brief HW_AGC_K_BLE4M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24              k_ble4m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_K_BLE4M_ADDR   0xD035017C
#define BESMDM_K_BLE_4M_MASK         ((uint32_t)0xFF000000)
#define BESMDM_K_BLE_4M_LSB          24
__STATIC_FORCEINLINE void besmdm_k_ble_4m_setf(uint8_t kble4m)
{
    ASSERT_ERR((((uint32_t)kble4m << 24) & ~((uint32_t)0xFF000000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_K_BLE4M_ADDR, BESMDM_K_BLE_4M_MASK, BESMDM_K_BLE_4M_LSB, kble4m);
}

/**
 * @brief HW_AGC_K_ANT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24          k_ant1m_pt1   0x0
 *  23:16          k_ant2m_pt1   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_K_ANT_ADDR    0xD03501B0
#define BESMDM_K_ANT_2M_PT_1_MASK   ((uint32_t)0x00FF0000)
#define BESMDM_K_ANT_2M_PT_1_LSB    16
#define BESMDM_K_ANT_1M_PT_1_MASK   ((uint32_t)0x000000FF)
#define BESMDM_K_ANT_1M_PT_1_LSB    0
__STATIC_FORCEINLINE void besmdm_hw_agc_k_ant_pack(uint8_t kant2mpt1, uint8_t kant1mpt1)
{
    ASSERT_ERR((((uint32_t)kant1mpt1 << 24) & ~((uint32_t)0xFF000000)) == 0);
    ASSERT_ERR((((uint32_t)kant2mpt1 << 16) & ~((uint32_t)0x00FF0000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_K_ANT_ADDR, BESMDM_K_ANT_2M_PT_1_MASK, BESMDM_K_ANT_2M_PT_1_LSB, kant2mpt1);
    REG_RPL_WR(BESMDM_HW_AGC_K_ANT_ADDR, BESMDM_K_ANT_1M_PT_1_MASK, BESMDM_K_ANT_1M_PT_1_LSB, kant1mpt1);
}

/**
 * @brief HW_AGC_CNT_LOCK_BT_BLE1M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  28:24      sel_fast_pwr_in   0x0
 *  23:00   cnt_lock_th_bt_ble1m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_CNT_LOCK_BT_BLE1M_ADDR   0xD0350104
#define BESMDM_SEL_FAST_PWR_IN_MASK            ((uint32_t)0x1F000000)
#define BESMDM_SEL_FAST_PWR_IN_LSB             24
#define BESMDM_CNT_LOCK_TH_BT_BLE_1M_MASK      ((uint32_t)0x00FFFFFF)
#define BESMDM_CNT_LOCK_TH_BT_BLE_1M_LSB       0
__STATIC_FORCEINLINE void besmdm_hw_agc_cnt_lock_bt_ble1m_pack(uint8_t selfastpwrin, uint32_t cntlockthbtble1m)
{
    ASSERT_ERR((((uint32_t)selfastpwrin << 24) & ~((uint32_t)0x1F000000)) == 0);
    ASSERT_ERR((((uint32_t)cntlockthbtble1m << 0) & ~((uint32_t)0x00FFFFFF)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_CNT_LOCK_BT_BLE1M_ADDR, BESMDM_SEL_FAST_PWR_IN_MASK, BESMDM_SEL_FAST_PWR_IN_LSB, selfastpwrin);
    REG_RPL_WR(BESMDM_HW_AGC_CNT_LOCK_BT_BLE1M_ADDR, BESMDM_CNT_LOCK_TH_BT_BLE_1M_MASK, BESMDM_CNT_LOCK_TH_BT_BLE_1M_LSB, cntlockthbtble1m);
}

/**
 * @brief HW_AGC_CNT_LOCK_BLE2M_BLELR register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  28:24      sel_fast_pwr_in   0x0
 *  23:00   cnt_lock_th_ble2m_blelr   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_CNT_LOCK_BLE2M_BLELR_ADDR   0xD0350108
#define BESMDM_SEL_FAST_PWR_IN_MASK               ((uint32_t)0x1F000000)
#define BESMDM_SEL_FAST_PWR_IN_LSB                24
#define BESMDM_CNT_LOCK_TH_BLE_2M_BLELR_MASK      ((uint32_t)0x00FFFFFF)
#define BESMDM_CNT_LOCK_TH_BLE_2M_BLELR_LSB       0
__STATIC_FORCEINLINE void besmdm_hw_agc_cnt_lock_ble2m_blelr_pack(uint8_t selfastpwrin, uint32_t cntlockthble2mblelr)
{
    ASSERT_ERR((((uint32_t)selfastpwrin << 24) & ~((uint32_t)0x1F000000)) == 0);
    ASSERT_ERR((((uint32_t)cntlockthble2mblelr << 0) & ~((uint32_t)0x00FFFFFF)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_CNT_LOCK_BLE2M_BLELR_ADDR, BESMDM_SEL_FAST_PWR_IN_MASK, BESMDM_SEL_FAST_PWR_IN_LSB, selfastpwrin);
    REG_RPL_WR(BESMDM_HW_AGC_CNT_LOCK_BLE2M_BLELR_ADDR, BESMDM_CNT_LOCK_TH_BLE_2M_BLELR_MASK, BESMDM_CNT_LOCK_TH_BLE_2M_BLELR_LSB, cntlockthble2mblelr);
}

/**
 * @brief HW_AGC_CNT_LOCK_BW2M_BW4M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00   cnt_lock_th_bt_bw2m_bw4m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_CNT_LOCK_BW2M_BW4M_ADDR    0xD03501A8
#define BESMDM_CNT_LOCK_TH_BT_BW_2M_BW_4M_MASK   ((uint32_t)0xFFFFFFFF)
#define BESMDM_CNT_LOCK_TH_BT_BW_2M_BW_4M_LSB    0
__STATIC_FORCEINLINE void besmdm_cnt_lock_th_bt_bw_2m_bw_4m_setf(uint32_t cntlockthbtbw2mbw4m)
{
    ASSERT_ERR((((uint32_t)cntlockthbtbw2mbw4m << 0) & ~((uint32_t)0xFFFFFFFF)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_CNT_LOCK_BW2M_BW4M_ADDR, BESMDM_CNT_LOCK_TH_BT_BW_2M_BW_4M_MASK, BESMDM_CNT_LOCK_TH_BT_BW_2M_BW_4M_LSB, cntlockthbtbw2mbw4m);
}

/**
 * @brief HW_AGC_CNT_LOCK_BT_BLE4M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  11:00   cnt_lock_th_bt_ble4m   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_CNT_LOCK_BT_BLE4M_ADDR   0xD03501AC
#define BESMDM_CNT_LOCK_TH_BT_BLE_4M_MASK      ((uint32_t)0x00000FFF)
#define BESMDM_CNT_LOCK_TH_BT_BLE_4M_LSB       0
__STATIC_FORCEINLINE void besmdm_cnt_lock_th_bt_ble_4m_setf(uint16_t cntlockthbtble4m)
{
    ASSERT_ERR((((uint32_t)cntlockthbtble4m << 0) & ~((uint32_t)0x00000FFF)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_CNT_LOCK_BT_BLE4M_ADDR, BESMDM_CNT_LOCK_TH_BT_BLE_4M_MASK, BESMDM_CNT_LOCK_TH_BT_BLE_4M_LSB, cntlockthbtble4m);
}

/**
 * @brief HW_AGC_CNT_LOCK_ANT1M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00   cnt_lock_th_ant1m_pt1_pt2   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_CNT_LOCK_ANT1M_ADDR          0xD0350174
#define BESMDM_CNT_LOCK_TH_ANT_1M_PT_1_PT_2_MASK   ((uint32_t)0xFFFFFFFF)
#define BESMDM_CNT_LOCK_TH_ANT_1M_PT_1_PT_2_LSB    0
__STATIC_FORCEINLINE void besmdm_cnt_lock_th_ant_1m_pt_1_pt_2_setf(uint32_t cntlockthant1mpt1pt2)
{
    ASSERT_ERR((((uint32_t)cntlockthant1mpt1pt2 << 0) & ~((uint32_t)0xFFFFFFFF)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_CNT_LOCK_ANT1M_ADDR, BESMDM_CNT_LOCK_TH_ANT_1M_PT_1_PT_2_MASK, BESMDM_CNT_LOCK_TH_ANT_1M_PT_1_PT_2_LSB, cntlockthant1mpt1pt2);
}

/**
 * @brief HW_AGC_CNT_LOCK_ANT2M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00   cnt_lock_th_ant2m_pt1_pt2   0x0
 * </pre>
 */
#define BESMDM_HW_AGC_CNT_LOCK_ANT2M_ADDR          0xD0350134
#define BESMDM_CNT_LOCK_TH_ANT_2M_PT_1_PT_2_MASK   ((uint32_t)0xFFFFFFFF)
#define BESMDM_CNT_LOCK_TH_ANT_2M_PT_1_PT_2_LSB    0
__STATIC_FORCEINLINE void besmdm_cnt_lock_th_ant_2m_pt_1_pt_2_setf(uint32_t cntlockthant2mpt1pt2)
{
    ASSERT_ERR((((uint32_t)cntlockthant2mpt1pt2 << 0) & ~((uint32_t)0xFFFFFFFF)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_CNT_LOCK_ANT2M_ADDR, BESMDM_CNT_LOCK_TH_ANT_2M_PT_1_PT_2_MASK, BESMDM_CNT_LOCK_TH_ANT_2M_PT_1_PT_2_LSB, cntlockthant2mpt1pt2);
}

/**
 * @brief HW_AGC_RRC_GAIN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     20   hwagc_rrc_gain_sel   0
 * </pre>
 */
#define BESMDM_HW_AGC_RRC_GAIN_ADDR   0xD03502C0
#define BESMDM_HWAGC_RRC_GAIN_SEL_BIT    ((uint32_t)0x00100000)
#define BESMDM_HWAGC_RRC_GAIN_SEL_POS    20
__STATIC_FORCEINLINE void besmdm_hwagc_rrc_gain_sel_setf(uint8_t hwagcrrcgainsel)
{
    ASSERT_ERR((((uint32_t)hwagcrrcgainsel << 20) & ~((uint32_t)0x00100000)) == 0);
    REG_RPL_WR(BESMDM_HW_AGC_RRC_GAIN_ADDR, BESMDM_HWAGC_RRC_GAIN_SEL_BIT, BESMDM_HWAGC_RRC_GAIN_SEL_POS, hwagcrrcgainsel);
}

/**
 * @brief MODEM_DELAY register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00          modem_delay   0x0
 * </pre>
 */
#define BESMDM_MODEM_DELAY_ADDR   0xD035020C
#define BESMDM_MODEM_DELAY_MASK   ((uint32_t)0xFFFFFFFF)
#define BESMDM_MODEM_DELAY_LSB    0
__STATIC_FORCEINLINE void besmdm_modem_delay_setf(uint32_t modemdelay)
{
    ASSERT_ERR((((uint32_t)modemdelay << 0) & ~((uint32_t)0xFFFFFFFF)) == 0);
    REG_RPL_WR(BESMDM_MODEM_DELAY_ADDR, BESMDM_MODEM_DELAY_MASK, BESMDM_MODEM_DELAY_LSB, modemdelay);
}

/**
 * @brief ONE_TONE_SET_GAIN register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:00    one_tone_set_gain   0x0
 * </pre>
 */
#define BESMDM_ONE_TONE_SET_GAIN_ADDR   0xD0350028
#define BESMDM_ONE_TONE_SET_GAIN_MASK   ((uint32_t)0xFFFFFFFF)
#define BESMDM_ONE_TONE_SET_GAIN_LSB    0
__STATIC_FORCEINLINE void besmdm_one_tone_set_gain_setf(uint32_t onetonesetgain)
{
    ASSERT_ERR((((uint32_t)onetonesetgain << 0) & ~((uint32_t)0xFFFFFFFF)) == 0);
    REG_RPL_WR(BESMDM_ONE_TONE_SET_GAIN_ADDR, BESMDM_ONE_TONE_SET_GAIN_MASK, BESMDM_ONE_TONE_SET_GAIN_LSB, onetonesetgain);
}

/**
 * @brief ONE_TONE_SET_FREQ register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  03:00    one_tone_set_freq   0x0
 * </pre>
 */
#define BESMDM_ONE_TONE_SET_FREQ_ADDR   0xD035002C
#define BESMDM_ONE_TONE_SET_FREQ_MASK   ((uint32_t)0x0000000F)
#define BESMDM_ONE_TONE_SET_FREQ_LSB    0
__STATIC_FORCEINLINE void besmdm_one_tone_set_freq_setf(uint8_t onetonesetfreq)
{
    ASSERT_ERR((((uint32_t)onetonesetfreq << 0) & ~((uint32_t)0x0000000F)) == 0);
    REG_RPL_WR(BESMDM_ONE_TONE_SET_FREQ_ADDR, BESMDM_ONE_TONE_SET_FREQ_MASK, BESMDM_ONE_TONE_SET_FREQ_LSB, onetonesetfreq);
}

/**
 * @brief MIX_MODE_CONFIG register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     30            start_mix   0
 * </pre>
 */
#define BESMDM_MIX_MODE_CONFIG_ADDR   0xD0350334
#define BESMDM_START_MIX_BIT    ((uint32_t)0x40000000)
#define BESMDM_START_MIX_POS    30
__STATIC_FORCEINLINE void besmdm_start_mix_setf(uint8_t startmix)
{
    ASSERT_ERR((((uint32_t)startmix << 30) & ~((uint32_t)0x40000000)) == 0);
    REG_RPL_WR(BESMDM_MIX_MODE_CONFIG_ADDR, BESMDM_START_MIX_BIT, BESMDM_START_MIX_POS, startmix);
}

/**
 * @brief ONE_TONE_ENERGY_GET_I register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  21:00        i_data_energy   0x0
 * </pre>
 */
#define BESMDM_ONE_TONE_ENERGY_GET_I_ADDR   0xD0350338
#define BESMDM_I_DATA_ENERGY_MASK   ((uint32_t)0x003FFFFF)
#define BESMDM_I_DATA_ENERGY_LSB    0
__STATIC_FORCEINLINE void besmdm_i_data_energy_setf(uint32_t idataenergy)
{
    ASSERT_ERR((((uint32_t)idataenergy << 0) & ~((uint32_t)0x003FFFFF)) == 0);
    REG_RPL_WR(BESMDM_ONE_TONE_ENERGY_GET_I_ADDR, BESMDM_I_DATA_ENERGY_MASK, BESMDM_I_DATA_ENERGY_LSB, idataenergy);
}

/**
 * @brief ONE_TONE_ENERGY_GET_Q register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  21:00        q_data_energy   0x0
 * </pre>
 */
#define BESMDM_ONE_TONE_ENERGY_GET_Q_ADDR   0xD035033C
#define BESMDM_Q_DATA_ENERGY_MASK   ((uint32_t)0x003FFFFF)
#define BESMDM_Q_DATA_ENERGY_LSB    0
__STATIC_FORCEINLINE void besmdm_q_data_energy_setf(uint32_t qdataenergy)
{
    ASSERT_ERR((((uint32_t)qdataenergy << 0) & ~((uint32_t)0x003FFFFF)) == 0);
    REG_RPL_WR(BESMDM_ONE_TONE_ENERGY_GET_Q_ADDR, BESMDM_Q_DATA_ENERGY_MASK, BESMDM_Q_DATA_ENERGY_LSB, qdataenergy);
}

/**
 * @brief FASTACK_TXEX register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     30         fastack_txex   0
 *     29           fastack_32   0
 *  28:26     corr_preamble_fa   0x0
 *     25      err_chk_mode_fa   0
 *     24       corr_new_en_fa   0
 *     23         corr_mode_fa   0
 *  22:16           corr_th_fa   0x0
 *  15:08      spwr_corr_th_fa   0x0
 *  07:00           spwr_th_fa   0x0
 * </pre>
 */
#define BESMDM_FASTACK_TXEX_ADDR   0xD0350228
#define BESMDM_FASTACK_TXEX_BIT        ((uint32_t)0x40000000)
#define BESMDM_FASTACK_TXEX_POS        30
#define BESMDM_FASTACK__32_BIT         ((uint32_t)0x20000000)
#define BESMDM_FASTACK__32_POS         29
#define BESMDM_CORR_PREAMBLE_FA_MASK   ((uint32_t)0x1C000000)
#define BESMDM_CORR_PREAMBLE_FA_LSB    26
#define BESMDM_ERR_CHK_MODE_FA_BIT     ((uint32_t)0x02000000)
#define BESMDM_ERR_CHK_MODE_FA_POS     25
#define BESMDM_CORR_NEW_EN_FA_BIT      ((uint32_t)0x01000000)
#define BESMDM_CORR_NEW_EN_FA_POS      24
#define BESMDM_CORR_MODE_FA_BIT        ((uint32_t)0x00800000)
#define BESMDM_CORR_MODE_FA_POS        23
#define BESMDM_CORR_TH_FA_MASK         ((uint32_t)0x007F0000)
#define BESMDM_CORR_TH_FA_LSB          16
#define BESMDM_SPWR_CORR_TH_FA_MASK    ((uint32_t)0x0000FF00)
#define BESMDM_SPWR_CORR_TH_FA_LSB     8
#define BESMDM_SPWR_TH_FA_MASK         ((uint32_t)0x000000FF)
#define BESMDM_SPWR_TH_FA_LSB          0
__STATIC_FORCEINLINE void besmdm_fastack_txex_pack(uint8_t fastacktxex, uint8_t fastack32, uint8_t corrpreamblefa, uint8_t errchkmodefa, uint8_t corrnewenfa, uint8_t corrmodefa, uint8_t corrthfa, uint8_t spwrcorrthfa, uint8_t spwrthfa)
{
    ASSERT_ERR((((uint32_t)fastacktxex << 30) & ~((uint32_t)0x40000000)) == 0);
    ASSERT_ERR((((uint32_t)fastack32 << 29) & ~((uint32_t)0x20000000)) == 0);
    ASSERT_ERR((((uint32_t)corrpreamblefa << 26) & ~((uint32_t)0x1C000000)) == 0);
    ASSERT_ERR((((uint32_t)errchkmodefa << 25) & ~((uint32_t)0x02000000)) == 0);
    ASSERT_ERR((((uint32_t)corrnewenfa << 24) & ~((uint32_t)0x01000000)) == 0);
    ASSERT_ERR((((uint32_t)corrmodefa << 23) & ~((uint32_t)0x00800000)) == 0);
    ASSERT_ERR((((uint32_t)corrthfa << 16) & ~((uint32_t)0x007F0000)) == 0);
    ASSERT_ERR((((uint32_t)spwrcorrthfa << 8) & ~((uint32_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint32_t)spwrthfa << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_FASTACK_TXEX_BIT, BESMDM_FASTACK_TXEX_POS, fastacktxex);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_FASTACK__32_BIT, BESMDM_FASTACK__32_POS, fastack32);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_CORR_PREAMBLE_FA_MASK, BESMDM_CORR_PREAMBLE_FA_LSB, corrpreamblefa);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_ERR_CHK_MODE_FA_BIT, BESMDM_ERR_CHK_MODE_FA_POS, errchkmodefa);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_CORR_NEW_EN_FA_BIT, BESMDM_CORR_NEW_EN_FA_POS, corrnewenfa);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_CORR_MODE_FA_BIT, BESMDM_CORR_MODE_FA_POS, corrmodefa);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_CORR_TH_FA_MASK, BESMDM_CORR_TH_FA_LSB, corrthfa);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_SPWR_CORR_TH_FA_MASK, BESMDM_SPWR_CORR_TH_FA_LSB, spwrcorrthfa);
    REG_RPL_WR(BESMDM_FASTACK_TXEX_ADDR, BESMDM_SPWR_TH_FA_MASK, BESMDM_SPWR_TH_FA_LSB, spwrthfa);
}

/**
 * @brief IQMCNTL_ADDR_CT register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     31      int_en_mismatch   0
 *     30          int_en_dump   0
 *     29       init_gain_mode   0
 *  27:14      int_tx_dccomp_i   0x0
 *  13:00      int_tx_dccomp_q   0x0
 * </pre>
 */
#define BESMDM_IQMCNTL_ADDR_CT_ADDR   0xD0350250
#define BESMDM_INT_EN_MISMATCH_BIT    ((uint32_t)0x80000000)
#define BESMDM_INT_EN_MISMATCH_POS    31
#define BESMDM_INT_EN_DUMP_BIT        ((uint32_t)0x40000000)
#define BESMDM_INT_EN_DUMP_POS        30
#define BESMDM_INIT_GAIN_MODE_BIT     ((uint32_t)0x20000000)
#define BESMDM_INIT_GAIN_MODE_POS     29
#define BESMDM_INT_TX_DCCOMP_I_MASK   ((uint32_t)0x0FFFC000)
#define BESMDM_INT_TX_DCCOMP_I_LSB    14
#define BESMDM_INT_TX_DCCOMP_Q_MASK   ((uint32_t)0x00003FFF)
#define BESMDM_INT_TX_DCCOMP_Q_LSB    0
__STATIC_FORCEINLINE void besmdm_iqmcntl_addr_ct_pack(uint8_t intenmismatch, uint8_t intendump, uint8_t initgainmode, uint16_t inttxdccompi, uint16_t inttxdccompq)
{
    ASSERT_ERR((((uint32_t)intenmismatch << 31) & ~((uint32_t)0x80000000)) == 0);
    ASSERT_ERR((((uint32_t)intendump << 30) & ~((uint32_t)0x40000000)) == 0);
    ASSERT_ERR((((uint32_t)initgainmode << 29) & ~((uint32_t)0x20000000)) == 0);
    ASSERT_ERR((((uint32_t)inttxdccompi << 14) & ~((uint32_t)0x0FFFC000)) == 0);
    ASSERT_ERR((((uint32_t)inttxdccompq << 0) & ~((uint32_t)0x00003FFF)) == 0);
    REG_RPL_WR(BESMDM_IQMCNTL_ADDR_CT_ADDR, BESMDM_INT_EN_MISMATCH_BIT, BESMDM_INT_EN_MISMATCH_POS, intenmismatch);
    REG_RPL_WR(BESMDM_IQMCNTL_ADDR_CT_ADDR, BESMDM_INT_EN_DUMP_BIT, BESMDM_INT_EN_DUMP_POS, intendump);
    REG_RPL_WR(BESMDM_IQMCNTL_ADDR_CT_ADDR, BESMDM_INIT_GAIN_MODE_BIT, BESMDM_INIT_GAIN_MODE_POS, initgainmode);
    REG_RPL_WR(BESMDM_IQMCNTL_ADDR_CT_ADDR, BESMDM_INT_TX_DCCOMP_I_MASK, BESMDM_INT_TX_DCCOMP_I_LSB, inttxdccompi);
    REG_RPL_WR(BESMDM_IQMCNTL_ADDR_CT_ADDR, BESMDM_INT_TX_DCCOMP_Q_MASK, BESMDM_INT_TX_DCCOMP_Q_LSB, inttxdccompq);
}

/**
 * @brief BW4M_RC_STEP register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  24:00         bw4m_rc_step   0x0
 * </pre>
 */
#define BESMDM_BW4M_RC_STEP_ADDR   0xD0350150
#define BESMDM_BW_4M_RC_STEP_MASK   ((uint32_t)0x01FFFFFF)
#define BESMDM_BW_4M_RC_STEP_LSB    0
__STATIC_FORCEINLINE void besmdm_bw_4m_rc_step_setf(uint32_t bw4mrcstep)
{
    ASSERT_ERR((((uint32_t)bw4mrcstep << 0) & ~((uint32_t)0x01FFFFFF)) == 0);
    REG_RPL_WR(BESMDM_BW4M_RC_STEP_ADDR, BESMDM_BW_4M_RC_STEP_MASK, BESMDM_BW_4M_RC_STEP_LSB, bw4mrcstep);
}

/**
 * @brief LR_PREAMBLE_DET_TH register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  28:00   lr_preamble_det_th   0x0
 * </pre>
 */
#define BESMDM_LR_PREAMBLE_DET_TH_ADDR   0xD035036C
#define BESMDM_LR_PREAMBLE_DET_TH_MASK   ((uint32_t)0x1FFFFFFF)
#define BESMDM_LR_PREAMBLE_DET_TH_LSB    0
__STATIC_FORCEINLINE void besmdm_lr_preamble_det_th_setf(uint32_t lrpreambledetth)
{
    ASSERT_ERR((((uint32_t)lrpreambledetth << 0) & ~((uint32_t)0x1FFFFFFF)) == 0);
    REG_RPL_WR(BESMDM_LR_PREAMBLE_DET_TH_ADDR, BESMDM_LR_PREAMBLE_DET_TH_MASK, BESMDM_LR_PREAMBLE_DET_TH_LSB, lrpreambledetth);
}

/**
 * @brief RXGFSK_RX_STO register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *     00     rxgfsk_rx_sto_en   0
 * </pre>
 */
#define BESMDM_RXGFSK_RX_STO_ADDR   0xD0350400
#define BESMDM_RXGFSK_RX_STO_EN_BIT    ((uint32_t)0x00000001)
#define BESMDM_RXGFSK_RX_STO_EN_POS    0
__STATIC_FORCEINLINE void besmdm_rxgfsk_rx_sto_en_setf(uint8_t rxgfskrxstoen)
{
    ASSERT_ERR((((uint32_t)rxgfskrxstoen << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(BESMDM_RXGFSK_RX_STO_ADDR, BESMDM_RXGFSK_RX_STO_EN_BIT, BESMDM_RXGFSK_RX_STO_EN_POS, rxgfskrxstoen);
}

/**
 * @brief RX_STARTUP_DELAY register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24   tx_startup_delay_bw4m   0x0
 *  23:16   tx_startup_delay_bw2m   0x0
 *  15:08   rx_startup_delay_4m   0x0
 *  07:00   rx_startup_delay_bw2m   0x0
 * </pre>
 */
#define BESMDM_RX_STARTUP_DELAY_ADDR   0xD0350438
#define BESMDM_TX_STARTUP_DELAY_BW_4M_MASK   ((uint32_t)0xFF000000)
#define BESMDM_TX_STARTUP_DELAY_BW_4M_LSB    24
#define BESMDM_TX_STARTUP_DELAY_BW_2M_MASK   ((uint32_t)0x00FF0000)
#define BESMDM_TX_STARTUP_DELAY_BW_2M_LSB    16
#define BESMDM_RX_STARTUP_DELAY__4M_MASK     ((uint32_t)0x0000FF00)
#define BESMDM_RX_STARTUP_DELAY__4M_LSB      8
#define BESMDM_RX_STARTUP_DELAY_BW_2M_MASK   ((uint32_t)0x000000FF)
#define BESMDM_RX_STARTUP_DELAY_BW_2M_LSB    0

__STATIC_FORCEINLINE void besmdm_rx_startup_delay_pack(uint8_t txstartupdelaybw4m, uint8_t txstartupdelaybw2m, uint8_t rxstartupdelay4m, uint8_t rxstartupdelaybw2m)
{
    ASSERT_ERR((((uint32_t)txstartupdelaybw4m << 24) & ~((uint32_t)0xFF000000)) == 0);
    ASSERT_ERR((((uint32_t)txstartupdelaybw2m << 16) & ~((uint32_t)0x00FF0000)) == 0);
    ASSERT_ERR((((uint32_t)rxstartupdelay4m << 8) & ~((uint32_t)0x0000FF00)) == 0);
    ASSERT_ERR((((uint32_t)rxstartupdelaybw2m << 0) & ~((uint32_t)0x000000FF)) == 0);
    REG_RPL_WR(BESMDM_RX_STARTUP_DELAY_ADDR, BESMDM_TX_STARTUP_DELAY_BW_4M_MASK, BESMDM_TX_STARTUP_DELAY_BW_4M_LSB, txstartupdelaybw4m);
    REG_RPL_WR(BESMDM_RX_STARTUP_DELAY_ADDR, BESMDM_TX_STARTUP_DELAY_BW_2M_MASK, BESMDM_TX_STARTUP_DELAY_BW_2M_LSB, txstartupdelaybw2m);
    REG_RPL_WR(BESMDM_RX_STARTUP_DELAY_ADDR, BESMDM_RX_STARTUP_DELAY__4M_MASK, BESMDM_RX_STARTUP_DELAY__4M_LSB, rxstartupdelay4m);
    REG_RPL_WR(BESMDM_RX_STARTUP_DELAY_ADDR, BESMDM_RX_STARTUP_DELAY_BW_2M_MASK, BESMDM_RX_STARTUP_DELAY_BW_2M_LSB, txstartupdelaybw4m);
}

/**
 * @brief OLD_DEMODULATE register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  13:10          ahi_lock_th   0x0
 *  06:02    ahi_compdl_offset   0x0
 *     00       old_demodulate   0
 * </pre>
 */
#define BESMDM_OLD_DEMODULATE_ADDR   0xD035000C
#define BESMDM_AHI_LOCK_TH_MASK         ((uint32_t)0x00003C00)
#define BESMDM_AHI_LOCK_TH_LSB          10
#define BESMDM_AHI_COMPDL_OFFSET_MASK   ((uint32_t)0x0000007C)
#define BESMDM_AHI_COMPDL_OFFSET_LSB    2
#define BESMDM_OLD_DEMODULATE_BIT       ((uint32_t)0x00000001)
#define BESMDM_OLD_DEMODULATE_POS       0
__STATIC_FORCEINLINE void besmdm_old_demodulate_pack(uint8_t ahilockth, uint8_t ahicompdloffset, uint8_t olddemodulate)
{
    ASSERT_ERR((((uint32_t)ahilockth << 10) & ~((uint32_t)0x00003C00)) == 0);
    ASSERT_ERR((((uint32_t)ahicompdloffset << 2) & ~((uint32_t)0x0000007C)) == 0);
    ASSERT_ERR((((uint32_t)olddemodulate << 0) & ~((uint32_t)0x00000001)) == 0);
    REG_RPL_WR(BESMDM_OLD_DEMODULATE_ADDR, BESMDM_AHI_LOCK_TH_MASK, BESMDM_AHI_LOCK_TH_LSB, ahilockth);
    REG_RPL_WR(BESMDM_OLD_DEMODULATE_ADDR, BESMDM_AHI_COMPDL_OFFSET_MASK, BESMDM_AHI_COMPDL_OFFSET_LSB, ahicompdloffset);
    REG_RPL_WR(BESMDM_OLD_DEMODULATE_ADDR, BESMDM_OLD_DEMODULATE_BIT, BESMDM_OLD_DEMODULATE_POS, olddemodulate);
}

/**
 * @brief INT_OSR12_BLE2M register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  01:00          osr12_ble2m   0x0
 * </pre>
 */
#define BESMDM_INT_OSR12_BLE2M_ADDR   0xD0350140
#define BESMDM_OSR_12_BLE_2M_MASK   ((uint32_t)0x00000003)
#define BESMDM_OSR_12_BLE_2M_LSB    0
__STATIC_FORCEINLINE void besmdm_osr_12_ble_2m_setf(uint8_t osr12ble2m)
{
    ASSERT_ERR((((uint32_t)osr12ble2m << 0) & ~((uint32_t)0x00000003)) == 0);
    REG_RPL_WR(BESMDM_INT_OSR12_BLE2M_ADDR, BESMDM_OSR_12_BLE_2M_MASK, BESMDM_OSR_12_BLE_2M_LSB, osr12ble2m);
}

/**
 * @brief INT_DIG_GAIN0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  29:20   int_dig_gain_scale2   0x0
 *  19:10   int_dig_gain_scale1   0x0
 *  09:00   int_dig_gain_scale0   0x0
 * </pre>
 */
#define BESMDM_INT_DIG_GAIN0_ADDR   0xD03500C4
#define BESMDM_INT_DIG_GAIN_SCALE_2_MASK   ((uint32_t)0x3FF00000)
#define BESMDM_INT_DIG_GAIN_SCALE_2_LSB    20
#define BESMDM_INT_DIG_GAIN_SCALE_1_MASK   ((uint32_t)0x000FFC00)
#define BESMDM_INT_DIG_GAIN_SCALE_1_LSB    10
#define BESMDM_INT_DIG_GAIN_SCALE_0_MASK   ((uint32_t)0x000003FF)
#define BESMDM_INT_DIG_GAIN_SCALE_0_LSB    0
__INLINE void besmdm_int_dig_gain0_pack(uint16_t intdiggainscale2, uint16_t intdiggainscale1, uint16_t intdiggainscale0)
{
    ASSERT_ERR((((uint32_t)intdiggainscale2 << 20) & ~((uint32_t)0x3FF00000)) == 0);
    ASSERT_ERR((((uint32_t)intdiggainscale1 << 10) & ~((uint32_t)0x000FFC00)) == 0);
    ASSERT_ERR((((uint32_t)intdiggainscale0 << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(BESMDM_INT_DIG_GAIN0_ADDR, BESMDM_INT_DIG_GAIN_SCALE_2_MASK, BESMDM_INT_DIG_GAIN_SCALE_2_LSB, intdiggainscale2);
    REG_RPL_WR(BESMDM_INT_DIG_GAIN0_ADDR, BESMDM_INT_DIG_GAIN_SCALE_1_MASK, BESMDM_INT_DIG_GAIN_SCALE_1_LSB, intdiggainscale1);
    REG_RPL_WR(BESMDM_INT_DIG_GAIN0_ADDR, BESMDM_INT_DIG_GAIN_SCALE_0_MASK, BESMDM_INT_DIG_GAIN_SCALE_0_LSB, intdiggainscale0);
}

/**
 * @brief INT_DIG_GAIN1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  29:20   int_dig_gain_scale5   0x0
 *  19:10   int_dig_gain_scale4   0x0
 *  09:00   int_dig_gain_scale3   0x0
 * </pre>
 */
#define BESMDM_INT_DIG_GAIN1_ADDR   0xD03500C8
#define BESMDM_INT_DIG_GAIN_SCALE_5_MASK   ((uint32_t)0x3FF00000)
#define BESMDM_INT_DIG_GAIN_SCALE_5_LSB    20
#define BESMDM_INT_DIG_GAIN_SCALE_4_MASK   ((uint32_t)0x000FFC00)
#define BESMDM_INT_DIG_GAIN_SCALE_4_LSB    10
#define BESMDM_INT_DIG_GAIN_SCALE_3_MASK   ((uint32_t)0x000003FF)
#define BESMDM_INT_DIG_GAIN_SCALE_3_LSB    0
__INLINE void besmdm_int_dig_gain1_pack(uint16_t intdiggainscale5, uint16_t intdiggainscale4, uint16_t intdiggainscale3)
{
    ASSERT_ERR((((uint32_t)intdiggainscale5 << 20) & ~((uint32_t)0x3FF00000)) == 0);
    ASSERT_ERR((((uint32_t)intdiggainscale4 << 10) & ~((uint32_t)0x000FFC00)) == 0);
    ASSERT_ERR((((uint32_t)intdiggainscale3 << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(BESMDM_INT_DIG_GAIN1_ADDR, BESMDM_INT_DIG_GAIN_SCALE_5_MASK, BESMDM_INT_DIG_GAIN_SCALE_5_LSB, intdiggainscale5);
    REG_RPL_WR(BESMDM_INT_DIG_GAIN1_ADDR, BESMDM_INT_DIG_GAIN_SCALE_4_MASK, BESMDM_INT_DIG_GAIN_SCALE_4_LSB, intdiggainscale4);
    REG_RPL_WR(BESMDM_INT_DIG_GAIN1_ADDR, BESMDM_INT_DIG_GAIN_SCALE_3_MASK, BESMDM_INT_DIG_GAIN_SCALE_3_LSB, intdiggainscale3);
}

/**
 * @brief INT_DIG_GAIN2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  19:10   int_dig_gain_scale7   0x0
 *  09:00   int_dig_gain_scale6   0x0
 * </pre>
 */
#define BESMDM_INT_DIG_GAIN2_ADDR   0xD03500CC
#define BESMDM_INT_DIG_GAIN_SCALE_7_MASK   ((uint32_t)0x000FFC00)
#define BESMDM_INT_DIG_GAIN_SCALE_7_LSB    10
#define BESMDM_INT_DIG_GAIN_SCALE_6_MASK   ((uint32_t)0x000003FF)
#define BESMDM_INT_DIG_GAIN_SCALE_6_LSB    0
__INLINE void besmdm_int_dig_gain2_pack(uint16_t intdiggainscale7, uint16_t intdiggainscale6)
{
    ASSERT_ERR((((uint32_t)intdiggainscale7 << 10) & ~((uint32_t)0x000FFC00)) == 0);
    ASSERT_ERR((((uint32_t)intdiggainscale6 << 0) & ~((uint32_t)0x000003FF)) == 0);
    REG_RPL_WR(BESMDM_INT_DIG_GAIN2_ADDR, BESMDM_INT_DIG_GAIN_SCALE_7_MASK, BESMDM_INT_DIG_GAIN_SCALE_7_LSB, intdiggainscale7);
    REG_RPL_WR(BESMDM_INT_DIG_GAIN2_ADDR, BESMDM_INT_DIG_GAIN_SCALE_6_MASK, BESMDM_INT_DIG_GAIN_SCALE_6_LSB, intdiggainscale6);
}

/**
 * @brief REG_58 register definition
 */
#define CMU_REG_58_ADDR   0xD0330058
#define CMU_REG_58_OFFSET 0x00000058
#define CMU_REG_58_INDEX  0x00000016
#define CMU_REG_58_RESET  0x00000000

/**
 * @brief REG_60 register definition
 */
#define CMU_REG_60_ADDR   0xD0330060
#define CMU_REG_60_OFFSET 0x00000060
#define CMU_REG_60_INDEX  0x00000018
#define CMU_REG_60_RESET  0x00000000

/**
 * @brief REG_64 register definition
 */
#define CMU_REG_64_ADDR   0xD0330064
#define CMU_REG_64_OFFSET 0x00000064
#define CMU_REG_64_INDEX  0x00000019
#define CMU_REG_64_RESET  0x00000000

#define BESMDM_INT_TX_IQCOMP_BASE   0xD0310000
