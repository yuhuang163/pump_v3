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

#ifdef LCDC_BASE

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

#define QSPI_CMD_TIMEOUT   5 /* ms */

#if defined(CHIP_BEST1502X) || defined(CHIP_BEST1600)
#define QSPI_FRAME_HEAD_MANUALLY
#endif

static struct LCDC_REG_T * const lcdc = (struct LCDC_REG_T *)LCDC_BASE;

static enum HAL_GPIO_PIN_T te_gpio_pin;
static HAL_LCDC_QSPI_TE_GPIO_IRQ_HANDLER te_gpio_hdlr;

POSSIBLY_UNUSED
static enum HAL_LCDC_QSPI_CFMT_E qspi_color_fmt = LCDC_QSPI_RGB888;
POSSIBLY_UNUSED
static enum HAL_LCDC_QSPI_LINE_MODE_E qspi_line_mode = LCDC_QSPI_4LINE;

static uint32_t qspi_cmd_mhz = 16;
static uint32_t qspi_mhz = 96;
static uint32_t lcdcpix_mhz = 24;

void hal_lcdc_qspi_initialize(enum HAL_LCDC_QSPI_CFMT_E color_fmt, enum HAL_LCDC_QSPI_LINE_MODE_E line_mode)
{
#ifdef LCDCPIX_MHZ
    lcdcpix_mhz = LCDCPIX_MHZ;
#endif
#ifdef LCDC_QSPI_MHZ
    qspi_mhz = LCDC_QSPI_MHZ;
#endif
    hal_cmu_lcdc_clock_enable(lcdcpix_mhz);
    hal_cmu_lcdc_qspi_clock_enable(qspi_mhz);
    hal_iomux_set_qspilcd0();

    lcdc->REG_1A8 = LCDC_CLK_INT_DIV(1);
    lcdc->REG_2A8 = LCDC_QSPI_USE0 | LCDC_SDR_EN | LCDC_CMDSIZE(0x1f);

    if (line_mode == LCDC_QSPI_1LINE) {
        lcdc->REG_1F8 = LCDC_SPI_HW_CTRL | LCDC_SMPN_SWAP_RB | LCDC_SPI_1LINE_MODE;
    } else if (line_mode == LCDC_QSPI_2LINE) {
        lcdc->REG_1F8 = LCDC_SPI_HW_CTRL | LCDC_SMPN_SWAP_RB | LCDC_SPI_2LINE_MODE;
        lcdc->REG_2B0 = LCDC_SPI_2LINE_MODE0;
    } else {
        lcdc->REG_1F8 = LCDC_SPI_HW_CTRL | LCDC_SMPN_SWAP_RB | LCDC_SPI_4LINE_MODE;
    }

    if (color_fmt == LCDC_QSPI_RGB565)
        lcdc->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                        LCDC_TXBITS(0xf) | LCDC_SCLKCNT(0x2);
    else /* RGB888 or RGB666, 24bits */
        lcdc->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                        LCDC_TXBITS(0x17) | LCDC_SCLKCNT(0x2);

    lcdc->REG_2A8 |= LCDC_READ_MUX_DATA0 | LCDC_QSPI_FAST_MODE;

#ifndef QSPI_FRAME_HEAD_MANUALLY
    if (line_mode == LCDC_QSPI_2LINE || line_mode == LCDC_QSPI_1LINE) {
        lcdc->REG_22C = LCDC_TXDATA_HEAD_24B(0x002c00) | LCDC_SPI_HEAD_TYPE(0x2) |
                        LCDC_USE_FRAME_HEAD;
    } else {
        uint32_t v = 0;
        if (color_fmt == LCDC_QSPI_RGB565)
            v = LCDC_SPI_4LINE_565;
        else if (color_fmt == LCDC_QSPI_RGB666)
            v = LCDC_SPI_4LINE_666;
        else
            v = LCDC_SPI_4LINE_888;
        lcdc->REG_22C = LCDC_TXDATA_HEAD_24B(0x002c00) | LCDC_SPI_HEAD_TYPE(0x2) |
                        LCDC_USE_FRAME_HEAD | v;
    }
#else
    if (line_mode == LCDC_QSPI_4LINE) {
        uint32_t v = 0;
        if (color_fmt == LCDC_QSPI_RGB565)
            v = LCDC_SPI_4LINE_565;
        else if (color_fmt == LCDC_QSPI_RGB666)
            v = LCDC_SPI_4LINE_666;
        else
            v = LCDC_SPI_4LINE_888;
        lcdc->REG_22C = v;
    }
#endif

    lcdc->REG_194 = LCDC_PXLMD(0x81);
    lcdc->REG_188 |= LCDC_IORDY_MSK;

    qspi_color_fmt = color_fmt;
    qspi_line_mode = line_mode;
}

void hal_lcdc_qspi_exit(void)
{
    hal_cmu_lcdc_qspi_clock_disable();
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
        lcdc->REG_1F8 &= ~LCDC_SMPN_SWAP_RB;
    else
        lcdc->REG_1F8 |= LCDC_SMPN_SWAP_RB;
}

void hal_lcdc_qspi_set_command_freq(uint32_t freqmhz)
{
    qspi_cmd_mhz = freqmhz;
}

void hal_lcdc_qspi_cs_sel(bool active)
{
    static bool cs_enabled = false;

    if (active) {
        cs_enabled = true;
        lcdc->REG_230 |= LCDC_SPI_XCS_DR;
        lcdc->REG_230 &= ~LCDC_SPI_XCS;
        hal_sys_timer_delay_us(1);
    } else if (cs_enabled) {
        hal_sys_timer_delay_us(1);
        lcdc->REG_230 |= LCDC_SPI_XCS;
        lcdc->REG_230 &= ~LCDC_SPI_XCS_DR;
        cs_enabled = false;
    }
}

#if defined(CHIP_BEST1502X) || defined(CHIP_BEST1600)

int hal_lcdc_qspi_send_command_list(uint8_t cmd, uint8_t param_count, uint8_t *param_list)
{
    uint32_t send = 0;
    uint32_t xfer, xfer_bit;
    uint32_t val;
    uint32_t regsave = lcdc->REG_1F8;

    /* set lcdc/qspi low freq for command transfer */
    hal_cmu_lcdc_set_pixmhz(qspi_cmd_mhz);

    lcdc->REG_1F8 = LCDC_SPI_1LINE_MODE;

    hal_lcdc_qspi_cs_sel(true);

    /* send command first */
    lcdc->REG_184 = 0x02000000 | (cmd << 8);
    hal_sys_timer_delay_us(4);
    lcdc->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(0) | LCDC_TXBITS(31) |
                    LCDC_SPI_ENA | LCDC_SPI_START;
    hal_sys_timer_delay_us(6);

    /* send paramters */
    while (send < param_count) {
        xfer = param_count - send > 4 ? 4 : param_count - send;
        xfer_bit = xfer * 8;
        val = 0;
        while (xfer --)
            val = param_list[send++] | (val << 8);

        lcdc->REG_184 = val;
        hal_sys_timer_delay_us(4);
        lcdc->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(0) | LCDC_TXBITS(xfer_bit - 1) |
            LCDC_SPI_ENA | LCDC_SPI_START;
        hal_sys_timer_delay_us(6);
    }

    hal_lcdc_qspi_cs_sel(false);

    lcdc->REG_1F8 = regsave;

    /* restore lcdc/qspi normal freq */
    hal_cmu_lcdc_set_pixmhz(lcdcpix_mhz);
    return 0;
}

int hal_lcdc_qspi_read_command(uint8_t cmd, uint8_t *read_buf, uint8_t len)
{
    uint32_t read_pos = 0;
    uint32_t xfer, xfer_bit;
    uint32_t val;
    uint32_t regsave = lcdc->REG_1F8;

    /* set lcdc/qspi low freq for command transfer */
    hal_cmu_lcdc_set_pixmhz(qspi_cmd_mhz);

    lcdc->REG_1F8 = LCDC_SPI_1LINE_MODE;

    hal_lcdc_qspi_cs_sel(true);

    /* send command first */
    lcdc->REG_184 = 0x03000000 | (cmd << 8);
    hal_sys_timer_delay_us(4);
    lcdc->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(0) | LCDC_TXBITS(31) |
                    LCDC_SPI_ENA | LCDC_SPI_START;
    hal_sys_timer_delay_us(6);

    /* read result to buffer */
    while (read_pos < len) {
        xfer = len - read_pos > 4 ? 4 : (len - read_pos);
        xfer_bit = xfer * 8;

        hal_sys_timer_delay_us(4);
        lcdc->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(xfer_bit) | LCDC_TXBITS(0) |
            LCDC_SPI_3W4WB | LCDC_SPI_ENA | LCDC_SPI_START;
        hal_sys_timer_delay_us(6);
        val = lcdc->REG_140;
        memcpy(read_buf + read_pos, &val, xfer);
        read_pos += xfer;
    }

    hal_lcdc_qspi_cs_sel(false);

    lcdc->REG_1F8 = regsave;

    /* restore lcdc/qspi normal clock */
    hal_cmu_lcdc_set_pixmhz(lcdcpix_mhz);
    return 0;
}

#ifdef QSPI_FRAME_HEAD_MANUALLY
static void hal_lcdc_qspi_send_frame_head()
{
    uint32_t regsave = lcdc->REG_1F8;

    lcdc->REG_1F8 = LCDC_SPI_1LINE_MODE;

    lcdc->REG_184 = 0x32002c00;
    hal_sys_timer_delay_us(4);
    lcdc->REG_180 = LCDC_SCLKCNT(2) | LCDC_RXBITS(0) | LCDC_TXBITS(31) |
                    LCDC_SPI_ENA | LCDC_SPI_START;

    hal_sys_timer_delay_us(6);

    lcdc->REG_1F8 = regsave;
}
#endif

int hal_lcdc_qspi_gen_start(void)
{
    hal_lcdc_qspi_cs_sel(true);

#ifdef QSPI_FRAME_HEAD_MANUALLY
    hal_lcdc_qspi_send_frame_head();
#endif

    if (qspi_color_fmt == LCDC_QSPI_RGB565)
        lcdc->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_TXBITS(0xf) | LCDC_SCLKCNT(0x2);
    else
        lcdc->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_TXBITS(0x17) | LCDC_SCLKCNT(0x2);

    hal_sys_timer_delay_us(1);

    lcdc->REG_224 = 0x1;
    return 0;
}

#else /* CHIP_BEST1502X || CHIP_BEST1600 */

static int hal_lcdc_qspi_wait_done(void)
{
    uint32_t start = hal_sys_timer_get();
    while (!(lcdc->REG_1C4 & LCDC_SPI_IRQ) &&
            TICKS_TO_MS(hal_sys_timer_get() - start) < QSPI_CMD_TIMEOUT) {
    }
    if (!(lcdc->REG_1C4 & LCDC_SPI_IRQ))
        return -1;
    lcdc->REG_1C4 &= ~LCDC_SPI_IRQ;
    return 0;
}

int hal_lcdc_qspi_send_command_list(uint8_t cmd, uint8_t param_count, uint8_t *param_list)
{
    int ret = 0;

    /* set qspi low freq for command transfer */
    hal_cmu_lcdc_set_qspimhz(qspi_cmd_mhz);

    /* clear qspi irq */
    lcdc->REG_1C4 &= ~LCDC_SPI_IRQ;

    lcdc->REG_284 = 0x02000000 | (cmd << 8);

    if (param_count > 0 && param_list) {
        uint32_t val, size, xfer, send;
        uint32_t *reg = (uint32_t *)&(lcdc->REG_288);

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
        lcdc->REG_280 = LCDC_CMD_ENA | LCDC_PARAM_ENA | LCDC_PARAM_CNT(param_count * 8 - 1);
    else
        lcdc->REG_280 = LCDC_CMD_ENA;

    if (hal_lcdc_qspi_wait_done())
        ret = -1;

    hal_sys_timer_delay(US_TO_TICKS(100));

    /* restore qspi normal freq */
    hal_cmu_lcdc_set_qspimhz(qspi_mhz);
    return ret;
}

int hal_lcdc_qspi_read_command(uint8_t cmd, uint8_t *read_buf, uint8_t len)
{
    int ret = 0;
    uint32_t rxbits, rxcnt;
    uint32_t val;

    /* set qspi low freq for command transfer */
    hal_cmu_lcdc_set_qspimhz(qspi_cmd_mhz);

    /* clear qspi irq */
    lcdc->REG_1C4 &= ~LCDC_SPI_IRQ;

    rxbits = (len >= 4) ? 0x1f : (len * 8 - 1);
    rxcnt = (len + 3) / 4 - 1;
    lcdc->REG_27C = LCDC_WAIT_RDCNT(0) | LCDC_READ_CMD_NUM(rxcnt);
    lcdc->REG_284 = 0x03000000 | (cmd << 8);
    lcdc->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                    LCDC_TXBITS(0x1f) | LCDC_SCLKCNT(0x2) |
                    LCDC_RXBITS(rxbits) | LCDC_SPI_READ_ENA;
    lcdc->REG_230 |= LCDC_SPI_XCS_DR; /* Enable CS mannually */
    lcdc->REG_280 = LCDC_CMD_ENA;

    if (hal_lcdc_qspi_wait_done()) {
        ret = -1;
        goto exit;
    }

    lcdc->REG_284 = 0;

    while (rxcnt-- > 0) {
        if (hal_lcdc_qspi_wait_done()) {
            ret = -1;
            goto exit;
        }
    }

    lcdc->REG_230 &= ~LCDC_SPI_XCS_DR;
    lcdc->REG_27C = 0;

    /* wait data ready in fifo */
    hal_sys_timer_delay(US_TO_TICKS(100));

    for (uint32_t pos = 0; pos < len; ) {
        uint8_t *p = (uint8_t *)&val;
        val = lcdc->REG_140;
        for (int i = rxbits / 8; i >= 0 && pos < len; i--)
            read_buf[pos++] = p[i];
    }

    if (qspi_color_fmt == LCDC_QSPI_RGB565)
        lcdc->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                        LCDC_TXBITS(0xf) | LCDC_SCLKCNT(0x2);
    else /* RGB888 or RGB666, 24bits */
        lcdc->REG_180 = LCDC_SPI_START | LCDC_SPI_ENA | LCDC_CLKINV |
                        LCDC_TXBITS(0x17) | LCDC_SCLKCNT(0x2);

exit:
    /* restore qspi normal freq */
    hal_cmu_lcdc_set_qspimhz(qspi_mhz);
    return ret;
}

int hal_lcdc_qspi_gen_start(void)
{
#ifdef QSPI_FRAME_HEAD_MANUALLY
    hal_lcdc_qspi_cs_sel(true);

    lcdc->REG_284 = 0x32002c00;
    lcdc->REG_280 = LCDC_CMD_ENA;
    hal_sys_timer_delay_us(100);
#endif

    lcdc->REG_224 = 0x1;
    return 0;
}

#endif /* !CHIP_BEST1502X && !CHIP_BEST1600  */

int hal_lcdc_qspi_send_command(uint8_t cmd)
{
    return hal_lcdc_qspi_send_command_list(cmd, 0, NULL);
}

int hal_lcdc_qspi_send_command_data(uint8_t cmd, uint32_t len, uint8_t p0,
                            uint8_t p1, uint8_t p2, uint8_t p3)
{
    uint8_t params[4] = {p0, p1, p2, p3};

    return hal_lcdc_qspi_send_command_list(cmd, len, params);
}

void hal_lcdc_qspi_te_gpio_irq_register(HAL_LCDC_QSPI_TE_GPIO_IRQ_HANDLER hdlr)
{
    te_gpio_hdlr = hdlr;
}

static void te_gpio_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    ASSERT(pin == te_gpio_pin, "%s: Bad te gpio: %u (expecting %u)", __func__, pin, te_gpio_pin);

    if (te_gpio_hdlr) {
        te_gpio_hdlr();
    }
}

void hal_lcdc_qspi_te_gpio_irq_enable(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = true,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = te_gpio_irq_handler,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    te_gpio_pin = (enum HAL_GPIO_PIN_T)hal_iomux_get_dsi_te_pin();
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {(enum HAL_IOMUX_PIN_T)te_gpio_pin, HAL_IOMUX_FUNC_AS_GPIO,
            HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(te_gpio_pin, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq(te_gpio_pin, &gpiocfg);
}

void hal_lcdc_qspi_te_gpio_irq_disable(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = false,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = NULL,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    hal_gpio_setup_irq(te_gpio_pin, &gpiocfg);
}

#endif /* LCDC_BASE */
