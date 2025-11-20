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
#include "plat_addr_map.h"
#if defined(DSI_BASE) && defined(LCDC_BASE)
#include "plat_types.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_dsi.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_lcdc.h"
#include "hal_psc.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "hal_trace.h"
#include "hal_dsi.h"
#include "reg_dsi.h"

//#define DSI_DEBUG
#ifdef  DSI_DEBUG
#define DSI_DBG(...)            TR_INFO( TR_MOD(HAL),  __VA_ARGS__)
#else
#define DSI_DBG(...)
#endif
#define DSI_INFO(...)           TR_ERROR( TR_MOD(HAL), __VA_ARGS__)

#define DSI_READ_TIMEOUT_MS     100
#define DSI_READ_MAX_LEN        128

#ifdef CSI_BASE
#define CHIP_HAS_MIPIPHY
#endif

#ifdef CHIP_HAS_MIPIPHY
#include "mipiphy.h"
#endif

static struct DSI_REG_T * const dsi_base[] =  {
    (struct DSI_REG_T *)DSI_BASE,
#ifdef DSI1_BASE
    (struct DSI_REG_T *)DSI1_BASE,
#endif
};

enum DSI_CMD_TYPE_T {
    CLN_ENT_LP = 0,
    CLN_EXIT_LP = 1,
    CLN_ENT_ULPS = 2,
    CLN_EXIT_ULPS = 3,
    DLN_LPT = 4,
    DLN_HST = 5,
    DLN_LPT_BTA = 6,
    DLN_HST_BTA = 7,
};

enum DSI_CMD_T {
    DSC_SHORT_WRITE = 0x05,
    DSC_SHORT_WRITE_1PAR = 0x15,
    DSC_LONG_WRITE = 0x39
};

struct hal_dsi_ctroller {
    bool inited;
    uint16_t width;
    enum HAL_DSI_MODE_E mode;
    enum HAL_DSI_LANE_E lane;
    enum hal_lcdc_dsi_cfmt_e color_fmt;

    bool in_ulps;
    HAL_DSI_ULPS_CB_HANDLER ulps_exit_hdlr;
    HWTIMER_ID ulps_exit_timer;
    bool in_sleep;

    bool teirq_enable;

    enum HAL_GPIO_PIN_T te_gpio_pin;
    HAL_DSI_TE_GPIO_IRQ_HANDLER te_gpio_hdlr;
};

static struct hal_dsi_ctroller dsi_ctrl[HAL_DSI_ID_QTY];

/**
 * @brief hal_dsi_lpbusy_timeout - wait lpcmd status busy bit with timout
 *
 * @param ticks : wait time
 * @return int : 0 success, -1 timeout
 */
__attribute__((unused)) static int hal_dsi_lpbusy_timeout(enum HAL_DSI_ID_E id, uint32_t ticks)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    uint32_t start = hal_sys_timer_get();

    while (hal_sys_timer_get() - start < ticks) {
        if (!(dsi->REG_014 & (1 << 4))) {
            return DSI_OK;
        }
    }

    return DSI_TIMEOUT;
}

#if !defined(CHIP_HAS_MIPIPHY)
/**
 * @brief hal_dsiphy_open - open dsi phy
 *
 */
static void hal_dsiphy_open(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];

    dsi->REG_070 |= REG_CHCK_EN_DSI;
    dsi->REG_070 = SET_BITFIELD(dsi->REG_070, REG_CHCK_DLY_TUNE_DSI, 0x4);
    dsi->REG_070 = SET_BITFIELD(dsi->REG_070, REG_CHCK_HSTX_DRV_DSI, 0xF);
    dsi->REG_070 = SET_BITFIELD(dsi->REG_070, REG_CHCK_LPTX_DRV_DSI, 0x4);

#if defined(CONFIG_SWAP_DSI_CLK_PN)
    dsi->REG_070 |= REG_CHCK_SWAP_PN_DSI;
#endif
    //dsi->REG_070 |= REG_PRECHARGE_LDO_CNT(0xf);
#if defined(CONFIG_SWAP_DSI_LANE0_PN)
    dsi->REG_068 |= REG_CH0_SWAP_PN_DSI;
#endif

#if defined(CONFIG_SWAP_DSI_LANE1_PN)
    dsi->REG_06C |= REG_CH1_SWAP_PN_DSI;
#endif
#if 0
    dsi->REG_060 = 0x7e1F;
    dsi->REG_064 = 0x924258;
#else
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_LDO0P4_RES, 0x8);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_LDO0P8_RES, 0x6);
#ifdef CHIP_BEST1502X
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_LDO1P2_RES, 0x8);
#else
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_LDO1P2_RES, 0x6);
#endif
#ifdef CHIP_BEST1600
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPCD_H, 0x6);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPCD_L, 0x6);
#else
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPCD_H, 0x4);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPCD_L, 0x2);
#endif
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPRX_H, 0x4);
    dsi->REG_064 = SET_BITFIELD(dsi->REG_064, REG_VB_LPRX_L, 0x4);

#ifdef CHIP_BEST1600
    dsi->REG_060 &= ~REG_SWAP_CKX8_DSI;
#else
    //dsi->REG_060 &= ~(REG_SWAP_CKX1_DSI | REG_SWAP_CKX8_DSI);
#endif
    dsi->REG_060 |= REG_EN_LPRX0_DSI;
    dsi->REG_060 = SET_BITFIELD(dsi->REG_060, REG_CLK_SWRC_DSI, 0x7);
    dsi->REG_060 |= REG_CLKGEN_PU_DSI;
    dsi->REG_060 |= REG_PU_CH_DSI;
    dsi->REG_060 |= REG_PU_LDO_DSI;
    dsi->REG_060 |= REG_DSI_PHY_RSTN;
    dsi->REG_060 |= REG_MASK_LP_RSTN;
    dsi->REG_060 |= REG_MASK_LP_LDO;
    dsi->REG_060 |= REG_CKX8_EN;

#ifdef CHIP_BEST1700
    /* 1700 DSI Clock Source:
     * 0: SYSPLL, 393Mhz;
     * 1: USBPLL, 480Mhz;
     * 2: OSCX4, 96Mhz;
     * 3: RCX8, 192Mhz;
     */
#if defined(DSI_USE_USBPLL)
    dsi->REG_060 |= REG_CKX8_SEL(1);
#elif defined(DSI_USE_OSCX4)
    dsi->REG_060 |= REG_CKX8_SEL(2);
#elif defined(DSI_USE_RCX8)
    dsi->REG_060 |= REG_CKX8_SEL(3);
#else
    dsi->REG_060 |= REG_CKX8_SEL(0);
#endif
#endif /* CHIP_BEST1700 */

#endif
    dsi->REG_060 |= REG_PRECHARGE_LDO_DSI;
    //hal_sys_timer_delay(MS_TO_TICKS(5));
    dsi->REG_060 &= ~REG_PRECHARGE_LDO_DSI;
}

static void hal_dsiphy_sleep(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_060 &= ~REG_DSI_PHY_RSTN;
    dsi->REG_060 &= ~REG_PU_LDO_DSI;
}

static void hal_dsiphy_wakeup(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_060 |= REG_PU_LDO_DSI;
    hal_sys_timer_delay_us(100);
    dsi->REG_060 |= REG_DSI_PHY_RSTN;
}
#endif

void hal_dsi_start_hs_clock(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_014 = 1;
}

void hal_dsi_stop_hs_clock(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_014 = 0;
}

static void dsi_init(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];

#if defined(CHIP_HAS_MIPIPHY)
    dsiphy_open(0, dsi_ctrl[id].lane);
#ifdef DSI1_BASE
    if (id == HAL_DSI_ID_1) {
        hal_cmu_dsi1_phy_reset_set();
        hal_cmu_dsi1_phy_reset_clear();
    } else
#endif
    {
        hal_cmu_dsi_phy_reset_set();
        hal_cmu_dsi_phy_reset_clear();
    }
#else
#ifdef DSI1_BASE
    /* DSI1 4-lane mode need DSI0 D0/D1 */
    if (dsi_ctrl[id].lane == DSI_LANE_4)
        hal_dsiphy_open(HAL_DSI_ID_0);
#endif
    hal_dsiphy_open(id);
#endif

    hal_dsi_stop_hs_clock(id);

    dsi->REG_000 = DSI_R_LPCD_DLY(1) | DSI_R_LANE_NUM(0) |
                   DSI_R_HSA_LP | DSI_R_HSE_EN | DSI_R_HFP_LP |
                   DSI_R_EOTP_EN | DSI_R_T_LPX(0x7) | DSI_R_CLK_T_LPX(0x3) |
                   DSI_R_CLK_POST(0x7);
    if (dsi_ctrl[id].lane == DSI_LANE_2) {
        dsi->REG_000 |= DSI_R_LANE_NUM(0x1);
    } else if (dsi_ctrl[id].lane == DSI_LANE_4) {
        dsi->REG_000 |= DSI_R_LANE_NUM(0x3);
        dsi->REG_074 |= DSI_4LANE_EN | DSI_4LANE_SWAP(0);
    }

    if (dsi_ctrl[id].mode != DSI_COMMAND_MODE) {
        dsi->REG_000 |= DSI_R_VIDEO_MODE;
        dsi->REG_000 &= ~(DSI_R_EOTP_EN | DSI_R_HSA_LP | DSI_R_HBP_LP);
        dsi->REG_048 &= ~(DSI_R_VIDEO_BIST_EN);
        if (dsi_ctrl[id].mode != DSI_VIDEO_PULSE_MODE)
            dsi->REG_000 &= ~(DSI_R_HSE_EN);
        if (dsi_ctrl[id].mode == DSI_VIDEO_BURST_MODE)
            dsi->REG_000 &= ~(DSI_R_HFP_LP);
    }

    dsi->REG_004 = DSI_R_HS_EXIT_TIME(0xb) | DSI_R_HS_PRPR_TIME(0x5) |
                   DSI_R_HS_ZERO_TIME(0xb) | DSI_R_HS_TRAIL_TIME(0xa) |
                   DSI_R_T_WAKEUP(0x207);
    dsi->REG_008 = DSI_R_CLK_EXIT_TIME(0xb) | DSI_R_CLK_PRPR_TIME(0x8) |
                   DSI_R_CLK_ZERO_TIME(0x1f) | DSI_R_CLK_TRAIL_TIME(0x5) |
                   DSI_R_CLK_T_WAKEUP(0x66);
    dsi->REG_00C = DSI_R_DTYPE(0x39) | DSI_R_VC_CH_ID(0x0) |
                   DSI_R_INPUT_TYPE(0x0) | DSI_R_DLANE_AD_TIME(0x12);

    dsi->REG_048 &= ~DSI_R_COLOR_BAR_WIDTH_MASK;
    dsi->REG_048 |= DSI_R_COLOR_BAR_WIDTH(0x100);

    dsi->REG_048 &= ~DSI_R_HSYNC_DLY_NUM_MASK;

#if defined(CHIP_BEST2009)
    dsi->REG_074 = DSI_R_HS_TRAIL_TIME2(6); // reg 74 is write only at 2009
#else
    dsi->REG_074 &= ~DSI_R_HS_TRAIL_TIME2_MASK;
    dsi->REG_074 |= DSI_R_HS_TRAIL_TIME2(5);
#endif

    dsi->REG_00C &= ~DSI_R_VIDEO_PACKET_LENTH_MASK;

    if (dsi_ctrl[id].color_fmt == DSI_RGB565) {
        dsi->REG_00C |= DSI_R_INPUT_TYPE(0x1) |
                        DSI_R_VIDEO_PACKET_LENTH(dsi_ctrl[id].width*2+6+1);
        dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x120);
    } else {
        dsi->REG_00C |= DSI_R_INPUT_TYPE(0x0) |
                        DSI_R_VIDEO_PACKET_LENTH(dsi_ctrl[id].width*3+6+1);
        dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x120);
    }

    if (dsi_ctrl[id].mode != DSI_COMMAND_MODE) {
        dsi->REG_00C &= ~DSI_R_DTYPE_MASK;
        dsi->REG_00C &= ~DSI_R_VIDEO_PACKET_LENTH_MASK;
        if (dsi_ctrl[id].color_fmt == DSI_RGB565) {
            dsi->REG_00C |= DSI_R_DTYPE(0x0e);
            dsi->REG_00C |= DSI_R_VIDEO_PACKET_LENTH(dsi_ctrl[id].width*2+6);
        } else {
            dsi->REG_00C |= DSI_R_DTYPE(0x3e);
            dsi->REG_00C |= DSI_R_VIDEO_PACKET_LENTH(dsi_ctrl[id].width*3+6);
        }
        dsi->REG_048 &= ~DSI_R_HSYNC_DLY_NUM_MASK;
        dsi->REG_048 |= DSI_R_HSYNC_DLY_NUM(0x4b4);
        dsi->REG_048 &= ~DSI_R_COLOR_BAR_WIDTH_MASK;
    }

#if !defined(CONFIG_SWAP_DSI_CLK_PN) && (defined(CHIP_BEST1502X) || defined(CHIP_BEST2003))
    dsi->REG_048 |= DSI_R_CL_DATA_SEL;
#endif

    //MASK(disable) all irq
    dsi->REG_010 = (dsi->REG_010 & ~(DSI_INTR_MASK_MASK | DSI_HS_CMD_RD_EN | DSI_LPRX_RD_EN))
                   | DSI_FLAG_MASK_MASK; //clear irq status
    dsi->REG_010 |= DSI_LPRX_RD_EN;// | (1<<31);//for DCS read cmd

#ifndef CHIP_BEST2009
    dsi->REG_060 &= ~REG_DSI_PHY_RSTN;
    hal_sys_timer_delay(MS_TO_TICKS(2));
    dsi->REG_060 |= REG_DSI_PHY_RSTN;
#endif
}

void hal_dsi_reset(enum HAL_DSI_ID_E id, uint16_t width)
{
    dsi_ctrl[id].width = width;

    dsi_init(id);

    if (dsi_ctrl[id].teirq_enable)
        hal_dsi_teirq_enable(id);
}

int hal_dsi_init(enum HAL_DSI_ID_E id, uint16_t width, enum HAL_DSI_MODE_E mode,
                  enum HAL_DSI_LANE_E lane, enum hal_lcdc_dsi_cfmt_e color_fmt)
{
    uint32_t pixmhz = 24;
    uint8_t pixbit = 24;
#ifdef DSIPLL_MHZ
    uint32_t dsimhz = DSIPLL_MHZ;
#elif defined(CHIP_BEST1700) && !defined(DSI_USE_USBPLL)
#if defined(DSI_USE_OSCX4)
    uint32_t dsimhz = 96;
    /* If DSI selects OSCX4, LCDC pixel clock should uses OSCX1 24Mhz */
#undef LCDCPIX_MHZ
#define LCDCPIX_MHZ   24
#elif defined(DSI_USE_RCX8)
    uint32_t dsimhz = 192;
#else
    uint32_t dsimhz = 393; /* DSI uses SYSPLL 393Mhz */
#endif
#else
    uint32_t dsimhz = 480;
#endif

    if (id >= HAL_DSI_ID_QTY)
        return DSI_FAILED;
    if (id == HAL_DSI_ID_0 && lane == DSI_LANE_4) /* DSI0 not support 4-lane */
        return DSI_FAILED;
    if (dsi_ctrl[id].inited)
        return DSI_OK;

    if (color_fmt == DSI_RGB565)
        pixbit = 16;

#if defined(LCDCPIX_MHZ)
    pixmhz = LCDCPIX_MHZ;
#else
    pixmhz = (dsimhz + pixbit - 1) / pixbit;
    if (mode == DSI_COMMAND_MODE)
        pixmhz *= 2;
    if (lane == DSI_LANE_2 || lane == DSI_LANE_4)
        pixmhz *= 2;
#endif

    if (mode == DSI_COMMAND_MODE) {
#ifdef DSI1_BASE
        if (id == HAL_DSI_ID_1)
            hal_iomux_set_dsi_te1();
        else
#endif
            hal_iomux_set_dsi_te();
    }

    hal_cmu_lcdc_clock_enable(pixmhz);
#ifdef DSI1_BASE
    if (id == HAL_DSI_ID_1) {
        hal_cmu_dsi1_clock_enable();
        /* DSI1 4-lane mode need DSI0 D0/D1, not need DSI0 pixel clock */
        if (lane == DSI_LANE_4) {
            hal_cmu_dsi_clock_enable();
            hal_cmu_clock_disable(HAL_CMU_MOD_Q_DIS_PIX);
        }
    } else
#endif
        hal_cmu_dsi_clock_enable();

    /* Get the real pixel freq */
    pixmhz = hal_cmu_lcdc_get_pixmhz();

    hal_lcdc_stby_vsync_calc(width, pixbit, pixmhz, dsimhz, lane);

    dsi_ctrl[id].width = width;
    dsi_ctrl[id].mode = mode;
    dsi_ctrl[id].lane = lane;
    dsi_ctrl[id].color_fmt = color_fmt;

    dsi_init(id);

    dsi_ctrl[id].inited = true;
    DSI_INFO("DSI%d init: width:%d, dsimhz:%d, lane:%d, pixbit:%d, pixmhz:%d",
              id, width, dsimhz, lane, pixbit, pixmhz);
    return DSI_OK;
}

void hal_dsi_exit(enum HAL_DSI_ID_E id)
{
    bool lcdcpix_disable = true;

    dsi_ctrl[id].inited = false;
    for (int i = 0; i < HAL_DSI_ID_QTY; i++) {
        if (dsi_ctrl[i].inited)
            lcdcpix_disable = false;
    }

#ifdef DSI1_BASE
    if (id == HAL_DSI_ID_1) {
        hal_cmu_dsi1_clock_disable();
        if (dsi_ctrl[id].lane == DSI_LANE_4)
            hal_cmu_dsi_clock_disable();
    } else
#endif
        hal_cmu_dsi_clock_disable();

    if (lcdcpix_disable)
        hal_cmu_lcdc_clock_disable();

    DSI_INFO("DSI%d exit.", id);
}

WEAK void hal_dsi_start(enum HAL_DSI_ID_E id)
{
}

int hal_dsi_read_cmd(enum HAL_DSI_ID_E id, uint8_t cmd, uint8_t *data, int len)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    volatile uint32_t val;
    uint8_t fifo_len;
    uint8_t rxbuf[DSI_READ_MAX_LEN];
    int rxlen = 0;
    int wordcnt = 0;
    uint32_t start = hal_sys_timer_get();

    if (len > (DSI_READ_MAX_LEN - 6)) {
        DSI_INFO("DSI%d read: FAILED, read size need < %d.", id, DSI_READ_MAX_LEN - 6);
        return DSI_FAILED;
    }

    dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_LP_RX_DONE_FLAG;

    /* send command */
    dsi->REG_018 = (len << 8) | 0x37;
    dsi->REG_014 =  (5 << 16) | (0xe1 << 8) | DLN_LPT;
    hal_sys_timer_delay_us(20);
    dsi->REG_018 = (cmd << 8)| 0x06;
    dsi->REG_014 = (5 << 16) | (0xe1 << 8) | DLN_LPT_BTA;
    hal_sys_timer_delay_us(10);

    /* read data */
    while (rxlen < DSI_READ_MAX_LEN) {
        val = dsi->REG_010;
        if (val & DSI_LPRX_SIZE_MASK) {
            rxbuf[rxlen++] = val & 0xff;
        } else if (val & (DSI_RX_ERR_FLAG)) {
            dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_RX_ERR_FLAG;
            DSI_INFO("DSI%d read: RX ERROR, val = 0x%x", id, val);
            return DSI_FAILED;
        } else if (val & (DSI_RX_TIMEOUT_FLAG)) {
            dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_RX_TIMEOUT_FLAG;
            DSI_INFO("DSI%d read: RX TIMEOUT, val = 0x%x", id, val);
            return DSI_TIMEOUT;
        } else if ((val & DSI_LP_RX_DONE_FLAG) && (val & DSI_LPRX_SIZE_MASK) == 0) {
            DSI_DBG("DSI%d read: RX DONE, val = 0x%x", id, val);
            break;
        } else if (hal_sys_timer_get() - start > MS_TO_TICKS(DSI_READ_TIMEOUT_MS)) {
            DSI_INFO("DSI%d read: RX TIMEOUT.", id);
            return DSI_TIMEOUT;
        }
        hal_sys_timer_delay_us(1);
    }

    /* clear fifo */
    fifo_len = GET_BITFIELD(val, DSI_LPRX_SIZE);
    if (fifo_len > 1) {
        DSI_DBG("DSI%d read: warning: fifo is not empty.", id);
        while (fifo_len > 1) {
            val = dsi->REG_010;
            fifo_len --;
        }
    }

    /* Read from DSI devices:
     * 0x02: Acknowledge with Error Report.
     * 0x1C: DCS Long Read Response.
     * 0x21: DCS Short Read Response, 1 byte.
     * 0x22: DCS Short Read Response, 2 bytes.
     * 0x1A: Generic Long Read Response.
     * 0x11: Generic Short Read Response, 1 byte.
     * 0x12: Generic Read Response, 2 bytes.
     */
    switch (rxbuf[1]) {
        case 0x11:
        case 0x21:
            if (rxlen < 3)
                goto error;
            len = 1;
            data[0] = rxbuf[2];
            break;
        case 0x12:
        case 0x22:
            if (rxlen < 4)
                goto error;
            len = 2;
            data[0] = rxbuf[2];
            data[1] = rxbuf[3];
            break;
        case 0x1A:
        case 0x1C:
            wordcnt = (int)((rxbuf[3] << 8) | rxbuf[2]);
            if (rxlen < wordcnt + 5)
                goto error;
            if (len > wordcnt) {
                DSI_DBG("DSI%d read: Warning, need %d bytes, got %d bytes.",
                            id, len, wordcnt);
                len = wordcnt;
            }
            for (int i = 0; i < len; i++)
                data[i] = rxbuf[i + 5];
            break;
        case 0x02:
            DSI_INFO("DSI%d read: Acknowledge with Error Report.", id);
            return DSI_ACK_ERROR;
        default:
            goto error;
    }

    DSI_DBG("DSI %d read: OK, %d bytes.", id, len);
    return len;

error:
    DSI_INFO("DSI%d read: got %d bytes error data:", id, rxlen);
    DUMP8("%02x ", rxbuf, rxlen);
    return DSI_DATA_ERROR;
}

int hal_dsi_send_cmd(enum HAL_DSI_ID_E id, uint8_t cmd)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_018 = (cmd << 8) | DSC_SHORT_WRITE;
    dsi->REG_014 = (5 << 16) | (0xe1 << 8) | DLN_LPT;
    return hal_dsi_lpbusy_timeout(id, US_TO_TICKS(30));
}

int hal_dsi_send_hscmd(enum HAL_DSI_ID_E id, uint8_t cmd)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    //in HS clk
    dsi->REG_014 = 0x1;
    //send cmd
    dsi->REG_018 = (cmd << 8) | DSC_SHORT_WRITE;
    dsi->REG_014 = (5 << 16) | (0xe1 << 8) | DLN_HST;
    return DSI_OK;
}

int hal_dsi_send_cmd_list(enum HAL_DSI_ID_E id, uint8_t cmd, uint8_t param_count,
                           uint8_t *params)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    int len = param_count + 1;

    /* Short write */
    if (param_count == 0) {
        return hal_dsi_send_cmd(id, cmd);
    }

    if (!params)
        return DSI_FAILED;

    /* Short write with 1 paramter */
    if (param_count == 1) {
        dsi->REG_018 = (params[0] << 16) | (cmd << 8) | DSC_SHORT_WRITE_1PAR;
        dsi->REG_014 = (5 << 16) | (0xe1 << 8) | DLN_LPT;
        return hal_dsi_lpbusy_timeout(id, US_TO_TICKS(50));
    }

    /* Long write with 2 parameters */
    if (param_count == 2) {
        dsi->REG_018 = (len << 8) | DSC_LONG_WRITE;
        dsi->REG_01C = cmd | (params[0] << 8) | (params[1] << 16);
        dsi->REG_014 = ((len + 4 + 3) << 16) | (0xe1 << 8) | DLN_LPT;
        return hal_dsi_lpbusy_timeout(id, US_TO_TICKS(70));
    }

    /* Long write with more parameters */
    dsi->REG_018 = (len << 8) | DSC_LONG_WRITE;
    for (int i = -1; i < param_count; i += 4) {
        uint32_t v = (i == -1) ? cmd : params[i];
        if (param_count - i >= 4) {
            v |= params[i + 1] << 8;
            v |= params[i + 2] << 16;
            v |= params[i + 3] << 24;
            dsi->REG_01C = v;
        } else if (param_count - i == 3) {
            v |= params[i + 1] << 8;
            v |= params[i + 2] << 16;
            dsi->REG_028 = v;
            break;
        } else if (param_count - i == 2) {
            v |= params[i + 1] << 8;
            dsi->REG_024 = v;
            break;
        } else if (param_count - i == 1) {
            dsi->REG_020 = v;
            break;
        }
    }
    dsi->REG_014 = ((len + 4 + 3) << 16) | (0xe1 << 8) | DLN_LPT;
    return hal_dsi_lpbusy_timeout(id, US_TO_TICKS(len * 20 + 10));
}

int hal_dsi_send_cmd_data(enum HAL_DSI_ID_E id, uint8_t cmd, uint32_t len, uint8_t p0,
                           uint8_t p1, uint8_t p2, uint8_t p3)
{
    uint8_t params[4] = {p0, p1, p2, p3};
    return hal_dsi_send_cmd_list(id, cmd, len, params);
}

int hal_dsi_send_long_array(enum HAL_DSI_ID_E id, uint8_t len, uint8_t *data)
{
    if (!data || len == 0)
        return DSI_FAILED;
    return hal_dsi_send_cmd_list(id, data[0], len - 1, &data[1]);
}

/**
 * hal_dsi_irqn - get dsi irq number
 */
int hal_dsi_irqn(enum HAL_DSI_ID_E id)
{
    return DSI_IRQn;
}

/**
 * hal_dsi_get_irqstate - get dsi irq state
 */
uint32_t hal_dsi_get_irqstate(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    return dsi->REG_010;
}

uint32_t hal_dsi_get_unov_flags(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    uint32_t ret = dsi->REG_030 & (DSI_OV_FLAG | DSI_UN_FLAG);
    dsi->REG_030 &= ~(DSI_OV_FLAG | DSI_UN_FLAG);
    return ret;
}

/**
 * hal_dsi_clear_teirq_flag - clear dsi te irq state
 */
void hal_dsi_clear_teirq_flag(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    /* write 1 to clear irq flag */
    dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_TE_INTR;
}

/**
 * hal_dsi_irq_disable - disable dsi all irqs
 */
void hal_dsi_irq_disable(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_010 = (dsi->REG_010 & ~(DSI_INTR_MASK_MASK | DSI_HS_CMD_RD_EN /*| DSI_LPRX_RD_EN*/)) //disalbe all irq
        | DSI_FLAG_MASK_MASK; //clear irq status
}

/**
 * hal_dsi_teirq_enable - enable dsi te irq
 */
void hal_dsi_teirq_enable(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_048 |= DSI_R_TE_EDGE_SEL;
    dsi->REG_010 |= DSI_INTR_MASK(DSI_TE_INTR>>DSI_FLAG_MASK_SHIFT);

    dsi_ctrl[id].teirq_enable = true;
}

/**
 * hal_dsi_teirq_disable - disable dsi te irq
 */
void hal_dsi_teirq_disable(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_048 &= ~DSI_R_TE_EDGE_SEL;
    dsi->REG_010 &= ~(DSI_INTR_MASK(DSI_TE_INTR>>DSI_FLAG_MASK_SHIFT));

    dsi_ctrl[id].teirq_enable = false;
}

/**
 * hal_dsi_irq_is_te - irq is te
 * @regval : dsi ISR value
 */
bool hal_dsi_irq_is_te(uint32_t regval)
{
   return !!(regval & DSI_TE_INTR);
}

/**
 * hal_dsi_irq_is_err - dsi is irq err
 */
bool hal_dsi_irq_is_err(uint32_t regval)
{
    return !!(regval & (DSI_RX_TIMEOUT_FLAG |
                        DSI_RX_ERR_FLAG |
                        DSI_BTA_FAIL_FLAG));
}

/**
 * hal_dsi_irq_clear_err - clear dsi error flag.
 */
void hal_dsi_irq_clear_err(enum HAL_DSI_ID_E id, uint32_t status)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) |
                               DSI_RX_TIMEOUT_FLAG |
                               DSI_RX_ERR_FLAG |
                               DSI_BTA_FAIL_FLAG;
}

/**
 * hal_dsi_enter_ulps_mode - enter ulps mode
 */
void hal_dsi_enter_ulps_mode(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    uint32_t time = hal_sys_timer_get();

    if (!dsi_ctrl[id].in_ulps) {
        DSI_DBG("hal_dsi_enter_ulps_mode");
        hal_sys_timer_delay_us(200);
        dsi->REG_010 = (dsi->REG_010 & ~DSI_FLAG_MASK_MASK) | DSI_CMD_DONE_FLAG;
        dsi->REG_014 = 0x0005e100;//dsi clk lane lp
        hal_sys_timer_delay_us(1);
        dsi->REG_014 = 0x00001e02;//clk lane enter ulps mode
        dsi->REG_014 = 0x00011e04;//data lane enter ulps mode
        while ((dsi->REG_010 & DSI_CMD_DONE_FLAG) == 0 &&
                hal_sys_timer_get() - time < MS_TO_TICKS(1)) {}
        dsi_ctrl[id].in_ulps = true;
    }
}

struct ulps_hdlr_param {
    enum HAL_DSI_ID_E id;
    void *param;
};
struct ulps_hdlr_param ulps_param[HAL_DSI_ID_QTY];

static void ulps_exit_handler(void *param)
{
    struct ulps_hdlr_param *p = (struct ulps_hdlr_param *)param;
    enum HAL_DSI_ID_E id = p->id;
    dsi_ctrl[id].in_ulps = false;
    if (dsi_ctrl[id].ulps_exit_hdlr != NULL)
        dsi_ctrl[id].ulps_exit_hdlr((uint32_t)p->param);
    dsi_ctrl[id].ulps_exit_hdlr = NULL;
}

void hal_dsi_exit_ulps_with_cb(enum HAL_DSI_ID_E id, HAL_DSI_ULPS_CB_HANDLER hdlr, void *param)
{
    struct DSI_REG_T * const dsi = dsi_base[id];

    ulps_param[id].id = id;
    ulps_param[id].param = param;

    if (dsi_ctrl[id].in_ulps) {
        DSI_DBG("exit ulps mode with callback.");
        dsi->REG_014 = 0x00001e04;//data lane exit ulps mode
        dsi->REG_014 = 0x00001e03;//clk lane exit ulps mode
        // T_wakeup >= 1ms
        if (hdlr != NULL) {
            dsi_ctrl[id].ulps_exit_hdlr = hdlr;
            if (dsi_ctrl[id].ulps_exit_timer == NULL)
                dsi_ctrl[id].ulps_exit_timer = hwtimer_alloc(ulps_exit_handler, &ulps_param[id]);
            hwtimer_update_then_start(dsi_ctrl[id].ulps_exit_timer,
                    ulps_exit_handler, &ulps_param[id], US_TO_HWTICKS(1200));
        } else {
            hal_sys_timer_delay_us(1500);
            dsi_ctrl[id].in_ulps = false;
        }
    }
}

/**
 * hal_dsi_exit_ulps_mode - exit ulps mode
 */
void hal_dsi_exit_ulps_mode(enum HAL_DSI_ID_E id)
{
    struct DSI_REG_T * const dsi = dsi_base[id];
    if (dsi_ctrl[id].in_ulps) {
        DSI_DBG("exit ulps mode.");
        dsi->REG_014 = 0x00001e04;//data lane exit ulps mode
        dsi->REG_014 = 0x00001e03;//clk lane exit ulps mode
        // T_wakeup >= 1ms
        hal_sys_timer_delay_us(1500);
        dsi_ctrl[id].in_ulps = false;
    }
}

void hal_dsi_sleep(enum HAL_DSI_ID_E id)
{
    if (dsi_ctrl[id].in_sleep)
        return;
#if defined(CHIP_HAS_MIPIPHY)
    dsiphy_sleep();
#else
    hal_dsiphy_sleep(id);
#endif
#ifdef DSI1_BASE
    if (id == HAL_DSI_ID_1) {
        hal_cmu_dsi1_sleep();
        if (dsi_ctrl[id].lane == DSI_LANE_4)
            hal_cmu_dsi_sleep();
    } else
#endif
        hal_cmu_dsi_sleep();
    dsi_ctrl[id].in_sleep = true;
}

#ifdef CHIP_BEST1600
extern bool pmu_rf_dsi_pll_calib(void);
#endif
void hal_dsi_wakeup(enum HAL_DSI_ID_E id)
{
    if (!dsi_ctrl[id].in_sleep)
        return;
#ifdef DSI1_BASE
    if (id == HAL_DSI_ID_1) {
        hal_cmu_dsi1_wakeup();
        if (dsi_ctrl[id].lane == DSI_LANE_4) {
            hal_cmu_dsi_wakeup();
            hal_cmu_clock_disable(HAL_CMU_MOD_Q_DIS_PIX);
        }
    } else
#endif
        hal_cmu_dsi_wakeup();
#ifdef CHIP_BEST1600
    pmu_rf_dsi_pll_calib();
#endif
#if defined(CHIP_HAS_MIPIPHY)
    dsiphy_wakeup();
#else
    hal_dsiphy_wakeup(id);
#endif
    dsi_ctrl[id].in_sleep = false;
}

void hal_dsi_te_gpio_irq_register(enum HAL_DSI_ID_E id, HAL_DSI_TE_GPIO_IRQ_HANDLER hdlr)
{
    dsi_ctrl[id].te_gpio_hdlr = hdlr;
}

static void hal_dsi_te_gpio_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    for (int i = 0; i < HAL_DSI_ID_QTY; i++) {
        if (dsi_ctrl[i].te_gpio_pin == pin && dsi_ctrl[i].te_gpio_hdlr) {
            dsi_ctrl[i].te_gpio_hdlr();
            return;
        }
    }
    ASSERT(0, "%s: Bad te gpio: %u!", __func__, pin);
}

void hal_dsi_te_gpio_irq_enable(enum HAL_DSI_ID_E id)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = true,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = hal_dsi_te_gpio_irq_handler,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

#ifdef DSI1_BASE
    if (id == HAL_DSI_ID_1)
        dsi_ctrl[id].te_gpio_pin = (enum HAL_GPIO_PIN_T)hal_iomux_get_dsi_te1_pin();
    else
#endif
        dsi_ctrl[id].te_gpio_pin = (enum HAL_GPIO_PIN_T)hal_iomux_get_dsi_te_pin();
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {(enum HAL_IOMUX_PIN_T)dsi_ctrl[id].te_gpio_pin,
            HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(dsi_ctrl[id].te_gpio_pin, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq(dsi_ctrl[id].te_gpio_pin, &gpiocfg);
}

void hal_dsi_te_gpio_irq_disable(enum HAL_DSI_ID_E id)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = false,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = NULL,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    hal_gpio_setup_irq(dsi_ctrl[id].te_gpio_pin, &gpiocfg);
}

#endif
