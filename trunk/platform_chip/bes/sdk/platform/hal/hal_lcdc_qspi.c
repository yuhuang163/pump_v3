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

#include "cmsis.h"
#include "plat_addr_map.h"

#if defined(LCDC_BASE) && !defined(LCDC_NO_QSPI)

#include "string.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_lcdc.h"
#include "hal_lcdc_qspi.h"
#include "hal_trace.h"
#include "reg_lcdc.h"
#include "hal_psc.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_chipid.h"

//#define QSPI_DEBUG
#ifdef  QSPI_DEBUG
#define QSPI_DBG(...)       TR_INFO( TR_MOD(HAL),  "//LCDC: " __VA_ARGS__)
#else
#define QSPI_DBG(...)
#endif
#define QSPI_INFO(...)      TR_INFO( TR_MOD(HAL), "//LCDC: " __VA_ARGS__)

#define QSPI_CMD_TIMEOUT    5 /* ms */

#if defined(CHIP_BEST1502X) || defined(CHIP_BEST1600) || defined(CHIP_BEST2003)
#define QSPI_VERSION_OLD
#define QSPI_FRAME_HEAD_MANUALLY
#endif

#ifndef LCDC_QSPI_BASE
#define LCDC_QSPI_BASE      LCDC_BASE
#endif

static struct LCDC_REG_T * const lcdc = (struct LCDC_REG_T *)LCDC_BASE;
static struct LCDC_REG_T * const qspi = (struct LCDC_REG_T *)LCDC_QSPI_BASE;

enum QSPI_CMD_T {
    QSPI_CMD_WRITE          = 0x02,
    QSPI_CMD_READ           = 0x03,

    /* Pixel write commands, with 24 bits (clk) address '0x002c00' */
    QSPI_PIX_WRITE_1LN      = QSPI_CMD_WRITE,
    QSPI_PIX_WRITE_2LN      = 0x72,
    QSPI_PIX_WRITE_4LN      = 0x32,
};

struct lcdc_qspi_ctrl_t {
    uint16_t pix_mhz;
    uint16_t bus_mhz;
    uint16_t cmd_mhz;

    enum HAL_LCDC_QSPI_CFMT_E color_fmt;
    enum HAL_LCDC_QSPI_LINE_MODE_E line_mode;

    bool dtr_enable;
    bool dual_enable;

    uint16_t bus_map;
};

struct lcdc_qspi_ctrl_t qspi_ctrl;

int hal_lcdc_qspi_init(enum HAL_LCDC_QSPI_ID_E id, struct HAL_LCDC_QSPI_CONFIG_T *config)
{
    if (id >= LCDC_QSPI_ID_QTY) {
        QSPI_INFO("Invalid QSPI BUS id. %d", id);
        return LCDC_FAILED;
    }
    if (!config) {
        QSPI_INFO("QSPI config is NULL.");
        return LCDC_FAILED;
    }
    if (qspi_ctrl.bus_map) {
        QSPI_INFO("QSPI is already inited, return.");
        return LCDC_OK;
    }

    qspi_ctrl.pix_mhz = config->pix_mhz;
    qspi_ctrl.bus_mhz = config->bus_mhz;
    qspi_ctrl.cmd_mhz = 16;
    qspi_ctrl.color_fmt = config->color_fmt;
    qspi_ctrl.line_mode = config->line_mode;
    qspi_ctrl.dtr_enable = config->dtr_enable;
    qspi_ctrl.dual_enable = config->dual_enable;
    qspi_ctrl.bus_map = 0;

    if (qspi_ctrl.pix_mhz == 0) {
#ifdef LCDCPIX_MHZ
        qspi_ctrl.pix_mhz = LCDCPIX_MHZ;
#else
        qspi_ctrl.pix_mhz = 24;
#endif
    }
    if (qspi_ctrl.bus_mhz == 0) {
#ifdef LCDC_QSPI_MHZ
        qspi_ctrl.bus_mhz = LCDC_QSPI_MHZ;
#else
        qspi_ctrl.bus_mhz = 96;
#endif
    }

    /* Config QSPI iomux */
    if (qspi_ctrl.dual_enable) {
#ifdef LCDC_HAS_DUAL_QSPI
        hal_iomux_set_qspilcd0();
        hal_iomux_set_qspilcd1();
        qspi_ctrl.bus_map = (1 << LCDC_QSPI_ID_1) | (1 << LCDC_QSPI_ID_1);
#else
        QSPI_INFO("DUAL-QSPI is not supported!");
        return LCDC_NOT_SUPPORT;
#endif
#ifdef LCDC_HAS_QSPI1
    } else if (id == LCDC_QSPI_ID_1) {
        hal_iomux_set_qspilcd1();
        qspi_ctrl.bus_map = (1 << LCDC_QSPI_ID_1);
#endif
    } else {
        hal_iomux_set_qspilcd0();
        qspi_ctrl.bus_map = (1 << LCDC_QSPI_ID_0);
    }

    /* Enable LCDC/QSPI clocks */
    hal_cmu_lcdc_clock_enable(qspi_ctrl.pix_mhz);
    hal_cmu_lcdc_qspi_clock_enable(qspi_ctrl.bus_mhz);

    qspi->REG_2A8 = LCDC_CMDSIZE(0x1f) | (qspi_ctrl.dtr_enable ? LCDC_DTR_EN : LCDC_SDR_EN);

    if (qspi_ctrl.dual_enable) {
        qspi->REG_2A8 |= LCDC_QSPI_USE0 | LCDC_QSPI_USE1 | LCDC_QSPI_DUAL_DISP;
#ifdef LCDC_HAS_QSPI1
    } else if (id == LCDC_QSPI_ID_1) {
        qspi->REG_2A8 |= LCDC_QSPI_USE1;
#endif
    } else {
        qspi->REG_2A8 |= LCDC_QSPI_USE0;
    }

    lcdc->REG_1A8 = LCDC_CLK_INT_DIV(1);

    if (qspi_ctrl.line_mode == LCDC_QSPI_1LINE) {
        qspi->REG_1F8 = LCDC_SPI_HW_CTRL | LCDC_SMPN_SWAP_RB | LCDC_SPI_1LINE_MODE;
    } else if (qspi_ctrl.line_mode == LCDC_QSPI_2LINE) {
        qspi->REG_1F8 = LCDC_SPI_HW_CTRL | LCDC_SMPN_SWAP_RB | LCDC_SPI_2LINE_MODE;
        qspi->REG_2B0 = LCDC_SPI_2LINE_MODE0;
    } else {
        qspi->REG_1F8 = LCDC_SPI_HW_CTRL | LCDC_SMPN_SWAP_RB | LCDC_SPI_4LINE_MODE;
    }

    if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB565)
        qspi->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                        LCDC_TXBITS(0xf) | LCDC_SCLKCNT(0x2);
    else /* RGB888 or RGB666, 24bits */
        qspi->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                        LCDC_TXBITS(0x17) | LCDC_SCLKCNT(0x2);

    qspi->REG_2A8 |= LCDC_READ_MUX_DATA0 | LCDC_QSPI_FAST_MODE;

#ifndef QSPI_FRAME_HEAD_MANUALLY
    if (qspi_ctrl.line_mode == LCDC_QSPI_1LINE) {
        qspi->REG_22C = LCDC_TXDATA_HEAD_24B(0x002c00) | LCDC_SPI_HEAD_TYPE(0x0) |
                        LCDC_USE_FRAME_HEAD;
    } else if (qspi_ctrl.line_mode == LCDC_QSPI_2LINE) {
        qspi->REG_22C = LCDC_TXDATA_HEAD_24B(0x002c00) | LCDC_SPI_HEAD_TYPE(0x3) |
                        LCDC_USE_FRAME_HEAD;
        qspi->REG_2A8 = SET_BITFIELD(qspi->REG_2A8, LCDC_HEAD_HIGH_8B, QSPI_PIX_WRITE_2LN);
        qspi->REG_230 = SET_BITFIELD(qspi->REG_230, LCDC_SPI_HEAD_NUM, 0x1f);
    } else if (qspi_ctrl.line_mode == LCDC_QSPI_MIP) {
        uint32_t v = 0;
        if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB565)
            v = LCDC_SPI_4LINE_565;
        else if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB666)
            v = LCDC_SPI_4LINE_666;
        else
            v = LCDC_SPI_4LINE_888;
        qspi->REG_22C = LCDC_TXDATA_HEAD_24B(0x10000C8) | LCDC_SPI_HEAD_TYPE(0x3) |
                        LCDC_USE_FRAME_HEAD | v;
        qspi->REG_2A8 = SET_BITFIELD(qspi->REG_2A8, LCDC_HEAD_HIGH_8B, 0x1);
        qspi->REG_230 = SET_BITFIELD(qspi->REG_230, LCDC_SPI_HEAD_NUM, 0x1B);
        qspi->REG_1F8 &= ~LCDC_SMPN_SWAP_RB;
    } else {
        uint32_t v = 0;
        if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB565)
            v = LCDC_SPI_4LINE_565;
        else if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB666)
            v = LCDC_SPI_4LINE_666;
        else
            v = LCDC_SPI_4LINE_888;
        qspi->REG_22C = LCDC_TXDATA_HEAD_24B(0x002c00) | LCDC_SPI_HEAD_TYPE(0x2) |
                        LCDC_USE_FRAME_HEAD | v;
    }
#else
    if (qspi_ctrl.line_mode == LCDC_QSPI_4LINE) {
        uint32_t v = 0;
        if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB565)
            v = LCDC_SPI_4LINE_565;
        else if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB666)
            v = LCDC_SPI_4LINE_666;
        else
            v = LCDC_SPI_4LINE_888;
        qspi->REG_22C = v;
    }
#endif

    /* enable write wait for dtr */
    if (qspi_ctrl.dtr_enable) {
        qspi->REG_2A8 |= LCDC_QSPI_VSYNC_START_SEL;
        qspi->REG_71C = SET_BITFIELD(qspi->REG_71C, LCDC_WRITE_DELAY_CNT, 640) | LCDC_WRITE_WAIT;
    }

    lcdc->REG_194 = LCDC_PXLMD(0x81);
    lcdc->REG_188 |= LCDC_IORDY_MSK;

    QSPI_INFO("QSPI%d %s: qspimhz:%d, lane:%d, colorfmt:%d, pixmhz:%d",
               id, qspi_ctrl.dtr_enable ? "DTR" : "SDR", qspi_ctrl.bus_mhz,
               qspi_ctrl.line_mode, qspi_ctrl.color_fmt, qspi_ctrl.pix_mhz);
    return LCDC_OK;
}

void hal_lcdc_qspi_exit(void)
{
    if (!qspi_ctrl.bus_map) {
        QSPI_INFO("QSPI is already closed.");
        return;
    }

    hal_cmu_lcdc_qspi_clock_disable();
    hal_cmu_lcdc_clock_disable();

    memset(&qspi_ctrl, 0, sizeof(qspi_ctrl));
    QSPI_INFO("QSPI exit.");
}

uint32_t hal_lcdc_qspi_get_mhz(void)
{
    return qspi_ctrl.bus_mhz;
}

uint32_t hal_lcdc_qspi_get_pixmhz(void)
{
    return qspi_ctrl.pix_mhz;
}

void hal_lcdc_qspi_sleep(void)
{
    hal_cmu_lcdc_qspi_sleep();
}

void hal_lcdc_qspi_wakeup(void)
{
    hal_cmu_lcdc_qspi_wakeup();
}

void hal_lcdc_qspi_swaprb_enable(bool enable)
{
    if (enable)
        qspi->REG_1F8 |= LCDC_SMPN_SWAP_RB;
    else
        qspi->REG_1F8 &= ~LCDC_SMPN_SWAP_RB;
}

void hal_lcdc_qspi_set_command_freq(uint32_t freqmhz)
{
    qspi_ctrl.cmd_mhz = freqmhz;
}

void hal_lcdc_qspi_cs_sel(bool active)
{
    static bool cs_enabled = false;

    if (active) {
        cs_enabled = true;
        qspi->REG_230 |= LCDC_SPI_XCS_DR;
        qspi->REG_230 &= ~LCDC_SPI_XCS;
        hal_sys_timer_delay_us(1);
    } else if (cs_enabled) {
        hal_sys_timer_delay_us(1);
        qspi->REG_230 |= LCDC_SPI_XCS;
        qspi->REG_230 &= ~LCDC_SPI_XCS_DR;
        cs_enabled = false;
    }
}

#ifdef QSPI_VERSION_OLD

int hal_lcdc_qspi_send_command_list(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd, uint8_t param_count,
                                    uint8_t *param_list)
{
    uint32_t send = 0;
    uint32_t xfer, xfer_bit;
    uint32_t val;
    uint32_t regsave = qspi->REG_1F8;

    /* set lcdc/qspi low freq for command transfer */
    hal_cmu_lcdc_set_pixmhz(qspi_ctrl.cmd_mhz);

    qspi->REG_1F8 = LCDC_SPI_1LINE_MODE;

    hal_lcdc_qspi_cs_sel(true);

    /* send command first */
    qspi->REG_184 = (QSPI_CMD_WRITE << 24) | (cmd << 8);
    hal_sys_timer_delay_us(4);
    qspi->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(0) | LCDC_TXBITS(31) |
                    LCDC_SPI_ENA | LCDC_SPI_START;
    hal_sys_timer_delay_us(6);

    /* send paramters */
    while (send < param_count) {
        xfer = param_count - send > 4 ? 4 : param_count - send;
        xfer_bit = xfer * 8;
        val = 0;
        while (xfer --)
            val = param_list[send++] | (val << 8);

        qspi->REG_184 = val;
        hal_sys_timer_delay_us(4);
        qspi->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(0) | LCDC_TXBITS(xfer_bit - 1) |
            LCDC_SPI_ENA | LCDC_SPI_START;
        hal_sys_timer_delay_us(6);
    }

    hal_lcdc_qspi_cs_sel(false);

    qspi->REG_1F8 = regsave;

    /* restore lcdc/qspi normal freq */
    hal_cmu_lcdc_set_pixmhz(qspi_ctrl.pix_mhz);
    QSPI_DBG("QSPI send cmd [%02x] with %d params: OK.", cmd, param_count);
    return LCDC_OK;
}

int hal_lcdc_qspi_read_command(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd, uint8_t *read_buf,
                               uint8_t len)
{
    uint32_t read_pos = 0;
    uint32_t xfer, xfer_bit;
    uint32_t val;
    uint32_t regsave = qspi->REG_1F8;

    /* set lcdc/qspi low freq for command transfer */
    hal_cmu_lcdc_set_pixmhz(qspi_ctrl.cmd_mhz);

    qspi->REG_1F8 = LCDC_SPI_1LINE_MODE;

    hal_lcdc_qspi_cs_sel(true);

    /* send command first */
    qspi->REG_184 = (QSPI_CMD_READ << 24) | (cmd << 8);
    hal_sys_timer_delay_us(4);
    qspi->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(0) | LCDC_TXBITS(31) |
                    LCDC_SPI_ENA | LCDC_SPI_START;
    hal_sys_timer_delay_us(6);

    /* read result to buffer */
    while (read_pos < len) {
        xfer = len - read_pos > 4 ? 4 : (len - read_pos);
        xfer_bit = xfer * 8;

        hal_sys_timer_delay_us(4);
        qspi->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(xfer_bit) | LCDC_TXBITS(0) |
            LCDC_SPI_3W4WB | LCDC_SPI_ENA | LCDC_SPI_START;
        hal_sys_timer_delay_us(6);
        val = qspi->REG_140;
        memcpy(read_buf + read_pos, &val, xfer);
        read_pos += xfer;
    }

    hal_lcdc_qspi_cs_sel(false);

    qspi->REG_1F8 = regsave;

    /* restore lcdc/qspi normal clock */
    hal_cmu_lcdc_set_pixmhz(qspi_ctrl.pix_mhz);
    QSPI_DBG("QSPI read cmd [%02x]: OK, %d bytes", cmd, len);
    return LCDC_OK;
}

#ifdef QSPI_FRAME_HEAD_MANUALLY
static void hal_lcdc_qspi_send_frame_head()
{
    uint32_t cmd;
    uint32_t regsave = qspi->REG_1F8;

    qspi->REG_1F8 = LCDC_SPI_1LINE_MODE;

    if (qspi_ctrl.line_mode == LCDC_QSPI_1LINE)
        cmd = QSPI_PIX_WRITE_1LN;
    else if (qspi_ctrl.line_mode == LCDC_QSPI_2LINE)
        cmd = QSPI_PIX_WRITE_2LN;
    else
        cmd = QSPI_PIX_WRITE_4LN;

    qspi->REG_184 = (cmd << 24) | 0x002c00;

    hal_sys_timer_delay_us(4);
    qspi->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(0) | LCDC_TXBITS(31) |
                    LCDC_SPI_ENA | LCDC_SPI_START;

    hal_sys_timer_delay_us(6);

    qspi->REG_1F8 = regsave;
}
#endif

int hal_lcdc_qspi_gen_start(void)
{
    hal_lcdc_qspi_cs_sel(true);

#ifdef QSPI_FRAME_HEAD_MANUALLY
    hal_lcdc_qspi_send_frame_head();
#endif

    if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB565)
        qspi->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_TXBITS(0xf) | LCDC_SCLKCNT(0x2);
    else
        qspi->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_TXBITS(0x17) | LCDC_SCLKCNT(0x2);

    hal_sys_timer_delay_us(1);

    lcdc->REG_224 = 0x1;
    return LCDC_OK;
}

#else /* QSPI_VERSION_OLD */

static int hal_lcdc_qspi_wait_done(void)
{
    uint32_t start = hal_sys_timer_get();
    while (!(qspi->REG_1C4 & LCDC_SPI_IRQ) &&
            TICKS_TO_MS(hal_sys_timer_get() - start) < QSPI_CMD_TIMEOUT) {
    }
    if (!(qspi->REG_1C4 & LCDC_SPI_IRQ))
        return -1;
    qspi->REG_1C4 &= ~LCDC_SPI_IRQ;
    return 0;
}

int hal_lcdc_qspi_send_command_list(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd, uint8_t param_count,
                                    uint8_t *param_list)
{
    int ret = LCDC_OK;

    if (!(qspi_ctrl.bus_map & (1 << id))) {
        QSPI_INFO("QSPI%d is not inited!", id);
        return LCDC_FAILED;
    }
    /* Switch bus for command write */
    if (qspi_ctrl.dual_enable) {
        qspi->REG_2A8 &= ~(LCDC_QSPI_USE0 | LCDC_QSPI_USE1 | LCDC_QSPI_DUAL_DISP);
        qspi->REG_2A8 |= (id == LCDC_QSPI_ID_0) ? LCDC_QSPI_USE0 : LCDC_QSPI_USE1;
    }

    /* set qspi low freq for command transfer */
    hal_cmu_lcdc_set_qspimhz(qspi_ctrl.cmd_mhz);

    /* clear qspi irq */
    qspi->REG_1C4 &= ~LCDC_SPI_IRQ;

    qspi->REG_284 = (QSPI_CMD_WRITE << 24) | (cmd << 8);

    if (param_count > 0 && param_list) {
        uint32_t val, size, xfer, send;
        uint32_t *reg = (uint32_t *)&(qspi->REG_288);

        /* Limit 256bits/32bytes parameters */
        if (param_count > 32)
            param_count = 32;

        for (size = (param_count + 3) / 4, send = 0; size; size--) {
            xfer = 4;
            val = 0;
            while (xfer--) {
                if (send < param_count)
                    val = param_list[send++] | (val << 8);
                else
                    val <<= 8;
            }
            *reg++ = val;
        }
    }

    if (param_count > 0 && param_list)
        qspi->REG_280 = LCDC_CMD_ENA | LCDC_PARAM_ENA | LCDC_PARAM_CNT(param_count * 8 - 1);
    else
        qspi->REG_280 = LCDC_CMD_ENA;

    if (hal_lcdc_qspi_wait_done())
        ret = LCDC_TIMEOUT;

    hal_sys_timer_delay(US_TO_TICKS(100));

    /* restore qspi normal freq */
    hal_cmu_lcdc_set_qspimhz(qspi_ctrl.bus_mhz);

    /* restore reg config for dual-qspi */
    if (qspi_ctrl.dual_enable) {
        qspi->REG_2A8 |= LCDC_QSPI_USE0 | LCDC_QSPI_USE1 | LCDC_QSPI_DUAL_DISP;
    }

    if (ret != LCDC_OK)
        QSPI_INFO("QSPI%d send cmd [%02x] with %d params: TIMEOUT!", id, cmd, param_count);
    else
        QSPI_DBG("QSPI%d send cmd [%02x] with %d params: OK.", id, cmd, param_count);
    return ret;
}

int hal_lcdc_qspi_read_command(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd, uint8_t *read_buf,
                               uint8_t len)
{
    int ret = LCDC_OK;
    uint32_t rxbits, rxcnt;
    uint32_t val;

    if (!(qspi_ctrl.bus_map & (1 << id))) {
        QSPI_INFO("QSPI%d is not inited!", id);
        return LCDC_FAILED;
    }
    /* Switch bus for data read */
    if (qspi_ctrl.dual_enable) {
        qspi->REG_2A8 &= ~(LCDC_QSPI_USE0 | LCDC_QSPI_USE1 | LCDC_QSPI_DUAL_DISP);
        qspi->REG_2A8 |= (id == LCDC_QSPI_ID_0) ? LCDC_QSPI_USE0 : LCDC_QSPI_USE1;
    }

    /* set qspi low freq for command transfer */
    hal_cmu_lcdc_set_qspimhz(qspi_ctrl.cmd_mhz);

    /* clear qspi irq */
    qspi->REG_1C4 &= ~LCDC_SPI_IRQ;

    rxbits = (len >= 4) ? 0x1f : (len * 8 - 1);
    rxcnt = (len + 3) / 4 - 1;
    qspi->REG_27C = LCDC_WAIT_RDCNT(0) | LCDC_READ_CMD_NUM(rxcnt);
    qspi->REG_284 = (QSPI_CMD_READ << 24) | (cmd << 8);
    qspi->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                    LCDC_TXBITS(0x1f) | LCDC_SCLKCNT(0x2) |
                    LCDC_RXBITS(rxbits) | LCDC_SPI_READ_ENA;
    qspi->REG_230 |= LCDC_SPI_XCS_DR; /* Enable CS mannually */
    qspi->REG_280 = LCDC_CMD_ENA;

    if (hal_lcdc_qspi_wait_done()) {
        ret = LCDC_TIMEOUT;
        goto exit;
    }

    qspi->REG_284 = 0;

    while (rxcnt-- > 0) {
        if (hal_lcdc_qspi_wait_done()) {
            ret = LCDC_TIMEOUT;
            goto exit;
        }
    }

    qspi->REG_230 &= ~LCDC_SPI_XCS_DR;
    qspi->REG_27C = 0;

    /* wait data ready in fifo */
    hal_sys_timer_delay(US_TO_TICKS(100));

    for (uint32_t pos = 0; pos < len; ) {
        uint8_t *p = (uint8_t *)&val;
        if (id == LCDC_QSPI_ID_0)
            val = qspi->REG_140;
        else
            val = qspi->REG_144;
        for (int i = rxbits / 8; i >= 0 && pos < len; i--)
            read_buf[pos++] = p[i];
    }

    if (qspi_ctrl.color_fmt == LCDC_QSPI_RGB565)
        qspi->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                        LCDC_TXBITS(0xf) | LCDC_SCLKCNT(0x2);
    else /* RGB888 or RGB666, 24bits */
        qspi->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                        LCDC_TXBITS(0x17) | LCDC_SCLKCNT(0x2);

exit:
    /* restore qspi normal freq */
    hal_cmu_lcdc_set_qspimhz(qspi_ctrl.bus_mhz);

    /* restore reg config for dual-qspi */
    if (qspi_ctrl.dual_enable) {
        qspi->REG_2A8 |= LCDC_QSPI_USE0 | LCDC_QSPI_USE1 | LCDC_QSPI_DUAL_DISP;
    }

    if (ret != LCDC_OK)
        QSPI_INFO("QSPI%d read cmd [%02x]: TIMEOUT!", id, cmd);
    else
        QSPI_DBG("QSPI%d read cmd [%02x]: OK, %d bytes.", id, cmd, len);
    return ret;
}

int hal_lcdc_qspi_gen_start(void)
{
    if (qspi_ctrl.bus_map == 0)
        return LCDC_FAILED;

#ifdef QSPI_FRAME_HEAD_MANUALLY
    uint32_t cmd;

    hal_lcdc_qspi_cs_sel(true);

    if (qspi_ctrl.line_mode == LCDC_QSPI_1LINE)
        cmd = QSPI_PIX_WRITE_1LN;
    else if (qspi_ctrl.line_mode == LCDC_QSPI_2LINE)
        cmd = QSPI_PIX_WRITE_2LN;
    else
        cmd = QSPI_PIX_WRITE_4LN;

    qspi->REG_284 = (cmd << 24) | 0x002c00;
    qspi->REG_280 = LCDC_CMD_ENA;
    hal_sys_timer_delay_us(100);
#endif

    lcdc->REG_224 = 0x1;
    return LCDC_OK;
}

#endif /* !QSPI_VERSION_OLD  */

int hal_lcdc_qspi_send_command(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd)
{
    return hal_lcdc_qspi_send_command_list(id, cmd, 0, NULL);
}

int hal_lcdc_qspi_send_command_data(enum HAL_LCDC_QSPI_ID_E id, uint8_t cmd, uint32_t len,
                                    uint8_t p0, uint8_t p1, uint8_t p2, uint8_t p3)
{
    uint8_t params[4] = {p0, p1, p2, p3};

    return hal_lcdc_qspi_send_command_list(id, cmd, len, params);
}

#endif /* LCDC_BASE */
