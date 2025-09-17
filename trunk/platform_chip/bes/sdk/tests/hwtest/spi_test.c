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
#ifdef SPI_TEST
/*
*****************************************************************************
WARNING: This example is tested that the general SPI can be both SPI master and slave.
         Not a special SPI slave in the spi_debug_slave_test.c example.
                                                     tanwenchen 2021-10-09
*****************************************************************************
*/
#include "cmsis.h"
#include "string.h"
#include "hal_spi.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"

#define     SPI_AS_MASTER
#define     SPI_TEST_MODE       1/*0: master read,slave write
                                   1: master write,slave read,2021-07-16
                                 */
//#define SPI_AIR780_TEST
//#define SPI_CS_SOFTWARE
//#define SPI_CLKOUT_ENABLE
//#define SPI_DMA_ENABLE
//#define SPI_3WIRE_LCD_TEST

//Do not modify the following statement
#ifdef SPI_AS_MASTER
    #define     SPI_ID_SELECT       0   //0:spi0,1:spilcd,2021-06-28
    #define     SPI_CS_GPIO         HAL_GPIO_PIN_P0_6   //Note: You need to check or modify it during use
#else
    #define     SPI_ID_SELECT       1   //0:spi0,1:spilcd,2021-06-28
    #define     SPI_CS_GPIO         HAL_GPIO_PIN_P0_6   //Note: You need to check or modify it during use
#endif

#define     SPI_CMD_READ        0xA1
#define     SPI_CMD_WRITE       0xA2
#define     SPI_CMD_DUMMY       0xFF
#define     SPI_DELAY_MS(ms)    hal_sys_timer_delay(MS_TO_TICKS(ms))

POSSIBLY_UNUSED static int32_t spi_write(uint8_t *pbuf, uint16_t len, uint8_t enable_dma)
{
    int32_t ret = 0;

#ifdef SPI_CS_SOFTWARE
    hal_gpio_pin_clr(SPI_CS_GPIO);
#endif
#if (SPI_ID_SELECT == 0)
    if (enable_dma) {
        ret = hal_spi_dma_send(pbuf, len, NULL);
    } else {
        ret = hal_spi_send(pbuf, len);
    }
#elif (SPI_ID_SELECT == 1)
    if (enable_dma) {
        ret = hal_spilcd_dma_send(pbuf, len, NULL);
    } else {
        ret = hal_spilcd_send(pbuf, len);
    }
#endif

#ifdef SPI_CS_SOFTWARE
    hal_gpio_pin_set(SPI_CS_GPIO);
#endif
    return ret;
}

POSSIBLY_UNUSED static int32_t spi_read_master(uint8_t *cmd, uint8_t *pbuf, uint16_t len, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint8_t outBuf[len + 1];

#ifdef SPI_CS_SOFTWARE
    hal_gpio_pin_clr(SPI_CS_GPIO);
#endif
#if (SPI_ID_SELECT == 0)
    if (enable_dma) {
        ret = hal_spi_dma_recv(cmd, outBuf, len + 1, NULL);
    } else {
        ret = hal_spi_recv(cmd, outBuf, len + 1);
    }
#elif (SPI_ID_SELECT == 1)
    if (enable_dma) {
        ret = hal_spilcd_dma_recv(cmd, outBuf, len + 1, NULL);
    } else {
        ret = hal_spilcd_recv(cmd, outBuf, len + 1);
    }
#endif

#ifdef SPI_CS_SOFTWARE
    hal_gpio_pin_set(SPI_CS_GPIO);
#endif
    memcpy(pbuf, outBuf + 1, len); //remove head

    return ret;
}

POSSIBLY_UNUSED static int32_t spi_read_slave(uint8_t *cmd, uint8_t *pbuf, uint16_t len, uint8_t enable_dma)
{
    int32_t ret = 0;

#ifdef SPI_CS_SOFTWARE
    hal_gpio_pin_clr(SPI_CS_GPIO);
#endif
#if (SPI_ID_SELECT == 0)
    if (enable_dma) {
        ret = hal_spi_dma_recv(cmd, pbuf, len, NULL);
    } else {
        ret = hal_spi_recv(cmd, pbuf, len);
    }
#elif (SPI_ID_SELECT == 1)
    if (enable_dma) {
        ret = hal_spilcd_dma_recv(cmd, pbuf, len, NULL);
    } else {
        ret = hal_spilcd_recv(cmd, pbuf, len);
    }
#endif

#ifdef SPI_CS_SOFTWARE
    hal_gpio_pin_set(SPI_CS_GPIO);
#endif
    return ret;
}

#ifdef SPI_3WIRE_LCD_TEST
#define LCD_RS_GPIO         HAL_GPIO_PIN_P0_4
#define LCD_SCK_GPIO        HAL_GPIO_PIN_P0_5
#define LCD_CS_GPIO         HAL_GPIO_PIN_P0_6
#define LCD_SDA_GPIO        HAL_GPIO_PIN_P0_7
POSSIBLY_UNUSED static int32_t spi_write_reg(uint8_t reg)
{
    uint8_t buf = reg;

    hal_gpio_pin_clr(LCD_RS_GPIO);
    return spi_write(&buf, 1, 0);
}

POSSIBLY_UNUSED static int32_t spi_write_dat(uint8_t data)
{
    uint8_t buf = data;

    hal_gpio_pin_set(LCD_RS_GPIO);
    return spi_write(&buf, 1, 0);
}

/* Currently all chips cannot meet the demand */
POSSIBLY_UNUSED static int32_t spi_read_lcd(uint8_t cmd, uint8_t *pbuf, uint16_t len)
{
    int32_t ret = 0;
    uint16_t buf = cmd << 8;
    uint8_t outBuf[len + 1];

#ifdef SPI_CS_SOFTWARE
    hal_gpio_pin_clr(SPI_CS_GPIO);
#endif

    hal_gpio_pin_clr(LCD_RS_GPIO);
    ret = hal_spi_recv(&buf, outBuf, len + 1);

#ifdef SPI_CS_SOFTWARE
    hal_gpio_pin_set(SPI_CS_GPIO);
#endif
    memcpy(pbuf, outBuf + 1, len); //remove head

    return ret;
}

/*******************************************************************************
**********************************simulate spi**********************************
********************************************************************************/
POSSIBLY_UNUSED static void spi_init_simu(void)
{
    const struct HAL_IOMUX_PIN_FUNCTION_MAP spi_gpio[] = {
        {LCD_SCK_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
        {LCD_CS_GPIO,  HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {LCD_SDA_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(spi_gpio, ARRAY_SIZE(spi_gpio));
    hal_gpio_pin_set_dir(LCD_SCK_GPIO, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(LCD_CS_GPIO,  HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(LCD_SDA_GPIO, HAL_GPIO_DIR_OUT, 0);
}

POSSIBLY_UNUSED static int32_t spi_read_lcd_simu(uint8_t cmd, uint8_t *pbuf, uint16_t len)
{
    uint32_t i;

    if (pbuf == NULL) {
        return -1024;
    }
    //select device
    hal_gpio_pin_clr(LCD_CS_GPIO);

    //send command
    hal_gpio_pin_clr(LCD_RS_GPIO);
    hal_gpio_pin_set_dir(LCD_SDA_GPIO, HAL_GPIO_DIR_OUT, 0);
    for (i = 0; i < 8; i++) {
        hal_gpio_pin_clr(LCD_SCK_GPIO);
        hal_sys_timer_delay_us(1);
        if (cmd & 0x80) {
            hal_gpio_pin_set(LCD_SDA_GPIO);
        } else {
            hal_gpio_pin_clr(LCD_SDA_GPIO);
        }
        hal_gpio_pin_set(LCD_SCK_GPIO);
        hal_sys_timer_delay_us(1);
        cmd <<= 1;
    }
    hal_gpio_pin_clr(LCD_SCK_GPIO);

    //ready to read data
    hal_gpio_pin_set(LCD_RS_GPIO);
    hal_gpio_pin_set_dir(LCD_SDA_GPIO, HAL_GPIO_DIR_IN, 0);

    //generate a dummy clock cycle
    if (len > 1) {
        hal_gpio_pin_set(LCD_SCK_GPIO);
        hal_sys_timer_delay_us(1);
        hal_gpio_pin_clr(LCD_SCK_GPIO);
        hal_sys_timer_delay_us(1);
    }

    //read data
    for (int j = 0; j < len; j++) {
        pbuf[j] = 0;
        for (i = 0; i < 8; i++) {
            pbuf[j] <<= 1;
            hal_gpio_pin_set(LCD_SCK_GPIO);
            hal_sys_timer_delay_us(1);
            pbuf[j] |= hal_gpio_pin_get_val(LCD_SDA_GPIO);
            hal_gpio_pin_clr(LCD_SCK_GPIO);
            hal_sys_timer_delay_us(1);
        }
    }
    hal_gpio_pin_clr(LCD_SCK_GPIO);

    //deselect device
    hal_gpio_pin_set(LCD_CS_GPIO);

    return 0;
}

#endif

void spi_test(void)
{
    struct HAL_SPI_CFG_T spi_cfg;
    POSSIBLY_UNUSED uint32_t spi_tx_data = 0;
    POSSIBLY_UNUSED uint32_t spi_rx_data = 0;
    POSSIBLY_UNUSED int32_t ret_tx;
    POSSIBLY_UNUSED int32_t ret_rx;
#ifdef SPI_AIR780_TEST
    POSSIBLY_UNUSED uint8_t buffer[18] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R'};
#else
    POSSIBLY_UNUSED uint8_t buffer[5] = {SPI_CMD_DUMMY};
#endif
    POSSIBLY_UNUSED uint32_t val;
    uint32_t error = 0;

    TRACE(0, "   ");
#ifdef CHIP_BEST2003
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_390M);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
#endif
    TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);

#ifdef SPI_AS_MASTER
    TRACE(0, "------SPI master test started------");
#else
    TRACE(0, "-------SPI slave test started------");
#endif
    TRACE(1, "CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);

    memset(&spi_cfg, 0, sizeof(spi_cfg));
#ifdef SPI_AIR780_TEST
    spi_cfg.clk_delay_half = true;
    spi_cfg.clk_polarity = false;
#elif defined(SPI_3WIRE_LCD_TEST)
    spi_cfg.clk_delay_half = false;
    spi_cfg.clk_polarity = false;
#else
    spi_cfg.clk_delay_half = true;
    spi_cfg.clk_polarity = true;
#endif
#ifdef SPI_AS_MASTER
    spi_cfg.slave = false;
#else
    spi_cfg.slave = true;
#endif

#ifdef SPI_DMA_ENABLE
    spi_cfg.dma_rx = true;
    spi_cfg.dma_tx = true;
#else
    spi_cfg.dma_rx = false;
    spi_cfg.dma_tx = false;
#endif
    spi_cfg.rx_sep_line = false;
    spi_cfg.cs = 0;
#ifdef SPI_AS_MASTER
    //1.crystal clock source: maximum frequency 48M (OSCx2=48M/2=24, finally get 24M)
    //2.pll clock source     : maximum frequency 52M (PLL/2=52,finally get 52M)
#ifdef SPI_AIR780_TEST
    spi_cfg.rate = 1 * 1000 * 1000;
#else
    spi_cfg.rate = 6 * 1000 * 1000;
#endif
#else
    spi_cfg.rate = 20 * 1000 * 1000;//PERIPH_PLL_FREQ frequency is twice this frequency
#endif
    spi_cfg.rx_bits = 8;
    spi_cfg.tx_bits = 8;
    spi_cfg.rx_frame_bits = 0;

    TRACE(0, "  ");
    TRACE(1, "spi_cfg.clk_delay_half:%d", spi_cfg.clk_delay_half);
    TRACE(1, "spi_cfg.clk_polarity  :%d", spi_cfg.clk_polarity);
    TRACE(1, "spi_cfg.slave         :%d", spi_cfg.slave);
    TRACE(1, "spi_cfg.dma_rx        :%d", spi_cfg.dma_rx);
    TRACE(1, "spi_cfg.dma_tx        :%d", spi_cfg.dma_tx);
    TRACE(1, "spi_cfg.rx_sep_line   :%d", spi_cfg.rx_sep_line);
    TRACE(1, "spi_cfg.cs            :%d", spi_cfg.cs);
    TRACE(1, "spi_cfg.rate          :%d", spi_cfg.rate);
    TRACE(1, "spi_cfg.rx_bits       :%d", spi_cfg.rx_bits);
    TRACE(1, "spi_cfg.tx_bits       :%d", spi_cfg.tx_bits);
    TRACE(1, "spi_cfg.rx_frame_bits :%d", spi_cfg.rx_frame_bits);
    TRACE(0, "  ");

#if (SPI_ID_SELECT == 0)
    hal_iomux_set_spi();
#ifndef SPI_AS_MASTER
    hal_iomux_set_spi_slave();
#endif
    error = hal_spi_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spi open error:%d", __func__, __LINE__, error);
#elif (SPI_ID_SELECT == 1)
    hal_iomux_set_spilcd();
#ifndef SPI_AS_MASTER
    hal_iomux_set_spilcd_slave();
#endif
    error = hal_spilcd_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spilcd open error:%d", __func__, __LINE__, error);
#endif

#ifdef SPI_CLKOUT_ENABLE
    hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable((SPI_ID_SELECT == 0) ? HAL_CMU_CLOCK_OUT_MCU_SPI0 : HAL_CMU_CLOCK_OUT_MCU_SPI1);
    TRACE(0, "------SPI master clk clkout pin: gpioxx");
    TRACE(0, "  ");
#endif

#ifdef SPI_CS_SOFTWARE
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {SPI_CS_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(SPI_CS_GPIO, HAL_GPIO_DIR_OUT, 1);
#endif

#ifdef SPI_3WIRE_LCD_TEST
    const struct HAL_IOMUX_PIN_FUNCTION_MAP rs_gpio[] = {
        {LCD_RS_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(rs_gpio, ARRAY_SIZE(rs_gpio));
    hal_gpio_pin_set_dir(LCD_RS_GPIO, HAL_GPIO_DIR_OUT, 0);

    TRACE(0, "  ");
    TRACE(0, "------lcd initial start");
    /* Start Initial Sequence */
    spi_write_reg(0x11);//Sleep exit
    //SPI_DELAY_MS(20);
    spi_write_reg(0xB1);//ST7735R Frame Rate
    spi_write_dat(0x01);
    spi_write_dat(0x2C);
    spi_write_dat(0x2D);
    spi_write_reg(0xB2);
    spi_write_dat(0x01);
    spi_write_dat(0x2C);
    spi_write_dat(0x2D);
    spi_write_reg(0xB3);
    spi_write_dat(0x01);
    spi_write_dat(0x2C);
    spi_write_dat(0x2D);
    spi_write_dat(0x01);
    spi_write_dat(0x2C);
    spi_write_dat(0x2D);
    spi_write_reg(0xB4);//Column inversion
    spi_write_dat(0x07);
    spi_write_reg(0xC0);//ST7735R Power Sequence
    spi_write_dat(0xA2);
    spi_write_dat(0x02);
    spi_write_dat(0x84);
    spi_write_reg(0xC1);
    spi_write_dat(0xC5);
    spi_write_reg(0xC2);
    spi_write_dat(0x0A);
    spi_write_dat(0x00);
    spi_write_reg(0xC3);
    spi_write_dat(0x8A);
    spi_write_dat(0x2A);
    spi_write_reg(0xC4);
    spi_write_dat(0x8A);
    spi_write_dat(0xEE);
    spi_write_reg(0xC5);//VCOM
    spi_write_dat(0x0E);
    spi_write_reg(0x36);//MX, MY, RGB mode
    spi_write_dat(0xC8);
    spi_write_reg(0xe0);//ST7735R Gamma Sequence
    spi_write_dat(0x0f);
    spi_write_dat(0x1a);
    spi_write_dat(0x0f);
    spi_write_dat(0x18);
    spi_write_dat(0x2f);
    spi_write_dat(0x28);
    spi_write_dat(0x20);
    spi_write_dat(0x22);
    spi_write_dat(0x1f);
    spi_write_dat(0x1b);
    spi_write_dat(0x23);
    spi_write_dat(0x37);
    spi_write_dat(0x00);
    spi_write_dat(0x07);
    spi_write_dat(0x02);
    spi_write_dat(0x10);
    spi_write_reg(0xe1);
    spi_write_dat(0x0f);
    spi_write_dat(0x1b);
    spi_write_dat(0x0f);
    spi_write_dat(0x17);
    spi_write_dat(0x33);
    spi_write_dat(0x2c);
    spi_write_dat(0x29);
    spi_write_dat(0x2e);
    spi_write_dat(0x30);
    spi_write_dat(0x30);
    spi_write_dat(0x39);
    spi_write_dat(0x3f);
    spi_write_dat(0x00);
    spi_write_dat(0x07);
    spi_write_dat(0x03);
    spi_write_dat(0x10);
    spi_write_reg(0x2a);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x7f);
    spi_write_reg(0x2b);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x00);
    spi_write_dat(0x9f);
    spi_write_reg(0xF0);//Enable test command
    spi_write_dat(0x01);
    spi_write_reg(0xF6);//Disable ram power save mode
    spi_write_dat(0x00);
    spi_write_reg(0x3A);//65k mode
    spi_write_dat(0x05);
    spi_write_reg(0x29);//Display on
    TRACE(0, "------lcd initial end");

#if 0
    TRACE(0, "  ");
    TRACE(0, "=================================================");
    TRACE(0, "===============TX:8bit + RX:8bit=================");
    TRACE(0, "=================================================");
    spi_cfg.rx_bits = 8;
    spi_cfg.tx_bits = 8;
    spi_cfg.rx_frame_bits = 16;

    TRACE(1, "spi_cfg.rx_bits       :%d", spi_cfg.rx_bits);
    TRACE(1, "spi_cfg.tx_bits       :%d", spi_cfg.tx_bits);
    TRACE(1, "spi_cfg.rx_frame_bits :%d", spi_cfg.rx_frame_bits);
    TRACE(0, "------spi reopen start");
#if (SPI_ID_SELECT == 0)
    error = hal_spi_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spi open error:%d", __func__, __LINE__, error);
#elif (SPI_ID_SELECT == 1)
    error = hal_spilcd_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spilcd open error:%d", __func__, __LINE__, error);
#endif
    TRACE(0, "------spi reopen end");
#else
    spi_init_simu();
#endif

    while (1) {
#if 0
        spi_read_lcd(0xDA, buffer, 1);
#else
        TRACE(0, " ");
        spi_read_lcd_simu(0x0A, buffer, 1);
        TRACE(0, "spi 3-wire read RDDPM:0x%02X", buffer[0]);

        spi_read_lcd_simu(0x0B, buffer, 1);
        TRACE(0, "spi 3-wire read MADCTL:0x%02X", buffer[0]);

        spi_read_lcd_simu(0x0C, buffer, 1);
        TRACE(0, "spi 3-wire read COLMOD:0x%02X", buffer[0]);

        spi_read_lcd_simu(0x0D, buffer, 1);
        TRACE(0, "spi 3-wire read RDDIM:0x%02X", buffer[0]);

        spi_read_lcd_simu(0x0E, buffer, 1);
        TRACE(0, "spi 3-wire read RDDSM:0x%02X", buffer[0]);

        spi_read_lcd_simu(0x0F, buffer, 1);
        TRACE(0, "spi 3-wire read RDDSDR:0x%02X", buffer[0]);

        spi_read_lcd_simu(0xDA, buffer, 1);
        TRACE(0, "spi 3-wire read ID1:0x%02X", buffer[0]);

        spi_read_lcd_simu(0xDB, buffer, 1);
        TRACE(0, "spi 3-wire read ID2:0x%02X", buffer[0]);

        spi_read_lcd_simu(0xDC, buffer, 1);
        TRACE(0, "spi 3-wire read ID3:0x%02X", buffer[0]);

        spi_read_lcd_simu(0x04, buffer, 3);
        TRACE(0, "spi 3-wire read ID:0x%02X-0x%02X-0x%02X", buffer[0], buffer[1], buffer[2]);
        SPI_DELAY_MS(50);
#endif
    }
#endif

    while (1) {
#ifdef SPI_AS_MASTER
#if (SPI_TEST_MODE == 1)
#ifdef SPI_AIR780_TEST
        for (uint8_t j = 0; j < sizeof(buffer); j++) {
            ret_tx = spi_write(&buffer[j], 1, spi_cfg.dma_tx);
        }
        TRACE(2, "spi master tx data:%s,ret:%d,total tx size:%d", buffer, ret_tx, sizeof(buffer));
        SPI_DELAY_MS(50);
#else
        spi_tx_data++;
        val = __REV(spi_tx_data);
        buffer[0] = SPI_CMD_WRITE;
        memcpy((uint8_t *)buffer + 1, (uint8_t *)&val, sizeof(spi_tx_data));
        //DUMP8("%02x ",buffer, sizeof(buffer));
        ret_tx = spi_write(buffer, sizeof(buffer), spi_cfg.dma_tx);
        TRACE(2, "spi master tx data:0x%X,ret:%d,total tx size:%d", spi_tx_data, ret_tx, sizeof(buffer));
#endif
#else
        ret_rx = spi_read_master(&buffer[0], (uint8_t *)&spi_rx_data, sizeof(spi_rx_data), spi_cfg.dma_rx);
        spi_rx_data = __REV(spi_rx_data);
        TRACE(2, "spi master rx data:0x%X,ret:%d", spi_rx_data, ret_rx);
#endif
        SPI_DELAY_MS(50);
#else
#if (SPI_TEST_MODE == 1)
        ret_rx = spi_read_master(&buffer[0], (uint8_t *)&spi_rx_data, sizeof(spi_rx_data), spi_cfg.dma_rx);
        spi_rx_data = __REV(spi_rx_data);
        TRACE(2, "spi slave rx data:0x%X,ret:%d ", spi_rx_data, ret_rx);
#else
        spi_tx_data++;
        val = __REV(spi_tx_data);
        buffer[0] = SPI_CMD_WRITE;
        memcpy((uint8_t *)buffer + 1, (uint8_t *)&val, sizeof(spi_tx_data));
        //DUMP8("%02x ",buffer, sizeof(buffer));
        ret_tx = spi_write(buffer, sizeof(buffer), spi_cfg.dma_tx);
        TRACE(2, "spi slave tx data:0x%X,ret:%d ", spi_tx_data, ret_tx);
#endif
#endif
    }
}

#endif

