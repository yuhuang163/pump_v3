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
#ifdef SPI_DEBUG_SLAVE_TEST

/*****************************************************************************
WARNING: This example test is a spi debug slave.
         Not a general SPI slave in the spi_test.c example.
                                                     tanwenchen 2021-10-09
*****************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "hal_spi.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "crc32_c.h"
#include "cmsis.h"
#include "hal_psc.h"

#if 1
    #define SPI_DEBUG_SLAVE_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define SPI_DEBUG_SLAVE_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
    #define SPI_DEBUG_SLAVE_DUMP16(n, s, ...)     DUMP16(n, s, ##__VA_ARGS__)
#else
    #define SPI_DEBUG_SLAVE_TRACE(n, s, ...)
    #define SPI_DEBUG_SLAVE_DUMP8(n, s, ...)
    #define SPI_DEBUG_SLAVE_DUMP16(n, s, ...)
#endif

#if 1
    #define SPI_DEBUG_SLAVE_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define SPI_DEBUG_SLAVE_DELAY_US(us)          hal_sys_timer_delay_us(us)
#else
    #define SPI_DEBUG_SLAVE_DELAY_MS(ms)
    #define SPI_DEBUG_SLAVE_DELAY_US(us)
#endif

/****************************************************************************
Key macro: COMPILE_SPI_DEBUG_SLAVE_CODE
Open     : The slave code is compiled (default)
Closed   : The master code is compiled
****************************************************************************/
#define COMPILE_SPI_DEBUG_SLAVE_CODE
#define ENABLE_CONTINUOUS_TEST_MODE
//#define ENABLE_WIFI_RAM_TEST

/*
    All hosts so far cannot support the third-line spi debug slave,
    so do not open the following macros
*/
//#define ENABLE_3WIRE_SPI
//#define REMOVED_SPI_PIN             HAL_IOMUX_PIN_P1_6  //chip1306

#ifdef ENABLE_WIFI_RAM_TEST
    #define TEST_RAM_ADDR           0x82000000
#else
    #define TEST_RAM_ADDR           0x20020000
#endif
#define EN_SPI_3WIRE_MODE           0xFFFC0000
#define EN_SPI_CTNS_READ            0xFFFC0004
#define SPI_MASTER_ID_SELECT        0   //0:spi0,1:spilcd,2021-06-28
//#define SPI_ENABLE_DMA

#define SPI_DEBUG_SLAVE_ADDR_WR(a)  (((a)>>2)|(0<<30))
#define SPI_DEBUG_SLAVE_ADDR_RD(a)  (((a)>>2)|(1<<30))
#define SPI_DUMMY_DATA              0xDEADBEEF

#define SPI_BUF_SIZE                (48*1024)
static uint8_t spi_buf[SPI_BUF_SIZE + 4 + 4];// 4 byte addr,4byte status

POSSIBLY_UNUSED static int32_t spi_write(const uint8_t *pbuf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret = 0;

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
    return ret;
}

POSSIBLY_UNUSED static int32_t spi_read(const uint8_t *cmd, uint8_t *pbuf, uint16_t len, uint8_t enable_dma)
{
    int32_t ret = 0;

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

    return ret;
}

POSSIBLY_UNUSED static int32_t spi_debug_slave_write(uint32_t addr, const uint8_t *pbuf, uint32_t len, uint8_t enable_dma)
{
    uint32_t i = 0;
    uint32_t integer;
    uint32_t remainder;
    int32_t  ret = 0;
    uint32_t *ptxbuf1;
    uint8_t  *ptxbuf2;

    ASSERT(!(len % 4), "len must be an integer multiple of 4,len:%d", len);

    integer = len / SPI_BUF_SIZE;
    remainder = len % SPI_BUF_SIZE;

    for (i = 0; i < integer; i++) {
        ptxbuf1 = (uint32_t *)(spi_buf + i * SPI_BUF_SIZE);
        ptxbuf2 = spi_buf + i * SPI_BUF_SIZE;

        ptxbuf1[0] = SPI_DEBUG_SLAVE_ADDR_WR(addr + i * SPI_BUF_SIZE);
        memcpy(ptxbuf2 + sizeof(uint32_t), pbuf + i * SPI_BUF_SIZE, SPI_BUF_SIZE);
        ret = spi_write((const uint8_t *)ptxbuf2, sizeof(uint32_t) + SPI_BUF_SIZE, enable_dma);
        if (ret) {
            SPI_DEBUG_SLAVE_TRACE(3, "%s:%d,spi write error,ret:%d", __func__, __LINE__, ret);
            return ret;
        }
    }

    if (remainder) {
        ptxbuf1 = (uint32_t *)(spi_buf + i * SPI_BUF_SIZE);
        ptxbuf2 = spi_buf + i * SPI_BUF_SIZE;

        ptxbuf1[0] = SPI_DEBUG_SLAVE_ADDR_WR(addr + i * SPI_BUF_SIZE);
        memcpy(ptxbuf2 + sizeof(uint32_t), pbuf + i * SPI_BUF_SIZE, remainder);
        ret = spi_write((const uint8_t *)ptxbuf2, sizeof(uint32_t) + remainder, enable_dma);
        if (ret) {
            SPI_DEBUG_SLAVE_TRACE(3, "%s:%d,spi write error,ret:%d", __func__, __LINE__, ret);
            return ret;
        }
    }

    return ret;
}

POSSIBLY_UNUSED static int32_t spi_debug_slave_read_word(uint32_t addr, uint32_t *val, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint32_t txbuf[3];
    uint32_t *rxbuf = (uint32_t *)spi_buf;

    txbuf[0] = SPI_DEBUG_SLAVE_ADDR_RD(addr);
    txbuf[1] = SPI_DUMMY_DATA;//status
    txbuf[2] = SPI_DUMMY_DATA;//data

    ret = spi_read((const uint8_t *)txbuf, spi_buf, sizeof(txbuf), enable_dma);
    *val = rxbuf[2];

    return ret;
}

POSSIBLY_UNUSED static int32_t spi_debug_slave_read_word_special(uint32_t addr, uint32_t *val, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint32_t txbuf[2];
    uint32_t *rxbuf = (uint32_t *)spi_buf;

    txbuf[0] = SPI_DEBUG_SLAVE_ADDR_RD(addr);
    txbuf[1] = SPI_DUMMY_DATA;//result

    ret = spi_read((const uint8_t *)txbuf, spi_buf, sizeof(txbuf), enable_dma);
    *val = rxbuf[1];

    return ret;
}

POSSIBLY_UNUSED static int32_t spi_debug_slave_write_word_special(uint32_t addr, uint32_t val, uint8_t enable_dma)
{
    uint32_t *txbuf = (uint32_t *)spi_buf;

    txbuf[0] = SPI_DEBUG_SLAVE_ADDR_WR(addr);
    txbuf[1] = val;

    return spi_write((const uint8_t *)txbuf, 2 * 4, enable_dma);
}

POSSIBLY_UNUSED static int32_t spi_debug_slave_read_non_continuous(uint32_t addr, uint8_t *pbuf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint32_t i = 0;

    ASSERT(!(len % 4), "len must be an integer multiple of 4,len:%d", len);

    for (i = 0; i < len / 4; i++) {
        ret = spi_debug_slave_read_word(addr + i * 4, (uint32_t *)(pbuf + i * 4), enable_dma);
        if (ret) {
            SPI_DEBUG_SLAVE_TRACE(3, "%s:%d,spi read error,ret:%d", __func__, __LINE__, ret);
            return ret;
        }
    }

    return ret;
}

POSSIBLY_UNUSED static int32_t spi_debug_slave_read_continuous(uint32_t addr, uint8_t *pbuf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint32_t i = 0;
    uint32_t integer;
    uint32_t remainder;
    uint32_t txbuf[3];

    ASSERT(!(len % 4), "len must be an integer multiple of 4,len:%d", len);

    integer = len / SPI_BUF_SIZE;
    remainder = len % SPI_BUF_SIZE;

    for (i = 0; i < integer; i++) {
        txbuf[0] = SPI_DEBUG_SLAVE_ADDR_RD(addr + i * SPI_BUF_SIZE);
        txbuf[1] = SPI_DUMMY_DATA;//status
        txbuf[2] = SPI_DUMMY_DATA;//data

        ret = spi_read((const uint8_t *)txbuf, spi_buf, SPI_BUF_SIZE + 8, enable_dma);
        if (ret) {
            SPI_DEBUG_SLAVE_TRACE(3, "%s:%d,spi read error,ret:%d", __func__, __LINE__, ret);
            return ret;
        }

        memcpy(pbuf + i * SPI_BUF_SIZE, spi_buf + 8, SPI_BUF_SIZE);
    }

    if (remainder) {
        txbuf[0] = SPI_DEBUG_SLAVE_ADDR_RD(addr + i * SPI_BUF_SIZE);
        txbuf[1] = SPI_DUMMY_DATA;//status
        txbuf[2] = SPI_DUMMY_DATA;//data

        ret = spi_read((const uint8_t *)txbuf, spi_buf, remainder + 8, enable_dma);
        if (ret) {
            SPI_DEBUG_SLAVE_TRACE(3, "%s:%d,spi read error,ret:%d", __func__, __LINE__, ret);
            return ret;
        }

        memcpy(pbuf + i * SPI_BUF_SIZE, spi_buf + 8, remainder);
    }

    return ret;
}

#define CRC_SIZE    4
#define BUF_SIZE    (32768+CRC_SIZE)
POSSIBLY_UNUSED static uint8_t buf[BUF_SIZE];
POSSIBLY_UNUSED static void rand_generator(uint8_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint8_t)rand();
    }
}

#define HW_WRITE_REG(_Reg,_Value)   (*((volatile uint32_t*)(_Reg)) = (uint32_t)(_Value))
void spi_debug_slave_test(void)
{
#ifdef COMPILE_SPI_DEBUG_SLAVE_CODE
    SPI_DEBUG_SLAVE_TRACE(0, "   ");

#ifdef ENABLE_WIFI_RAM_TEST
#if defined(CHIP_BEST2002)
    /* start to reset wifi before init */
    //hal_cmu_wifi_reset_set();
    //wifidrv_delay(1);
    //HW_WRITE_REG(AON_CMU_BASE+SOFT_RSTN_CLR, AON_CMU_SOFT_RSTN_WF_CLR);
    //hal_cmu_reset_pulse(HAL_CMU_MOD_H_AH2H_WF);
    //hal_psc_wifi_enable();
    //HW_WRITE_REG(AON_CMU_BASE+TOP_CLK_ENABLE, AON_CMU_EN_CLK_TOP_PLLBB_WF_ENABLE);
    //HW_WRITE_REG(AON_CMU_BASE+TOP_CLK_ENABLE, AON_CMU_EN_CLK_WF_ENABLE);
    //HW_WRITE_REG(AON_CMU_BASE+PLL_ENABLE, AON_CMU_PU_PLLBB_ENABLE);

    /* start to reset wifi before init */
    hal_cmu_wifi_reset_set();
    SPI_DEBUG_SLAVE_DELAY_MS(1);
    HW_WRITE_REG(0x400800A8, (1 << 4));         //release wifi reset
    hal_cmu_reset_pulse(HAL_CMU_MOD_H_AH2H_WF); // reset ahb2wf bridge
    hal_psc_wifi_enable();
    HW_WRITE_REG(0x40080004, (1 << 8)); //en_clk_top_pllbb_wf
    HW_WRITE_REG(0x40080004, (1 << 13));//en_clk_wf
    HW_WRITE_REG(0x4008008C, (1 << 0)); //en_clk_top_pllbb_wf
#else
#error The chip does not support the test address!
#endif
#endif

    hal_iomux_set_spi_slave();
#ifdef ENABLE_CONTINUOUS_TEST_MODE
    uint32_t *pdata = ((uint32_t *)(TEST_RAM_ADDR));
    uint32_t sum;

    while (1) {
        sum = 0;
        for (uint32_t i = 0; i < (BUF_SIZE - CRC_SIZE) / sizeof(uint32_t); i++) {
            sum += *pdata;
            pdata++;
        }
        SPI_DEBUG_SLAVE_TRACE(0, "The SPI debug slave function has been opened, use SPI master to access the register");
        SPI_DEBUG_SLAVE_TRACE(0, "read/write start addr:0x%X, size:%d, sum:0x%X", TEST_RAM_ADDR, BUF_SIZE - CRC_SIZE, sum);
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }
#else
    uint8_t *p = ((uint8_t *)(TEST_RAM_ADDR));

    SPI_DEBUG_SLAVE_TRACE(0, "The SPI debug slave function has been opened, use SPI master to access the register");
    SPI_DEBUG_SLAVE_TRACE(1, "The 32 bytes of data starting from address 0x%08X are as follows:", TEST_RAM_ADDR);
    while (1) {
        SPI_DEBUG_SLAVE_DUMP8("%02X ", p, 32);
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }
#endif

#else
    struct HAL_SPI_CFG_T spi_cfg;
    uint32_t cnt = 0;
    uint32_t val = 0;
    int32_t ret;
    uint32_t error = 0;
    uint32_t tx_crc = 0;
    uint32_t rx_crc = 0;
    uint32_t error_cnt = 0;

    SPI_DEBUG_SLAVE_TRACE(0, "   ");
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    SPI_DEBUG_SLAVE_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SPI_DEBUG_SLAVE_TRACE(0, "-------SPI debug slave test started------");

    memset(&spi_cfg, 0, sizeof(spi_cfg));
    spi_cfg.clk_delay_half = true;
    spi_cfg.clk_polarity = true;
    spi_cfg.slave = false;
#ifdef SPI_ENABLE_DMA
    spi_cfg.dma_rx = true;
    spi_cfg.dma_tx = true;
#else
    spi_cfg.dma_rx = false;
    spi_cfg.dma_tx = false;
#endif
    spi_cfg.rx_sep_line = false;
    spi_cfg.cs = 0;
    spi_cfg.rate = 6 * 1000 * 1000;
    spi_cfg.rx_bits = 32;
    spi_cfg.tx_bits = 32;
    spi_cfg.rx_frame_bits = 0;

    SPI_DEBUG_SLAVE_TRACE(0, "  ");
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.clk_delay_half:%d", spi_cfg.clk_delay_half);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.clk_polarity  :%d", spi_cfg.clk_polarity);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.slave         :%d", spi_cfg.slave);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.dma_rx        :%d", spi_cfg.dma_rx);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.dma_tx        :%d", spi_cfg.dma_tx);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.rx_sep_line   :%d", spi_cfg.rx_sep_line);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.cs            :%d", spi_cfg.cs);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.rate          :%d", spi_cfg.rate);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.rx_bits       :%d", spi_cfg.rx_bits);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.tx_bits       :%d", spi_cfg.tx_bits);
    SPI_DEBUG_SLAVE_TRACE(1, "spi_cfg.rx_frame_bits :%d", spi_cfg.rx_frame_bits);
    SPI_DEBUG_SLAVE_TRACE(0, "  ");

#if (SPI_MASTER_ID_SELECT==0)
    hal_iomux_set_spi();
    error = hal_spi_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spi open error:%d", __func__, __LINE__, error);
#elif (SPI_MASTER_ID_SELECT==1)
    hal_iomux_set_spilcd();
    error = hal_spilcd_open(&spi_cfg);
    ASSERT(!error, "%s:%d,spilcd open error:%d", __func__, __LINE__, error);
#endif

    srand(hal_sys_timer_get());
    SPI_DEBUG_SLAVE_TRACE(0, "  ");
    spi_debug_slave_read_word_special(EN_SPI_3WIRE_MODE, &val, spi_cfg.dma_rx);
    SPI_DEBUG_SLAVE_TRACE(2, "---read 3wire spi reg[0x%X]      :%d->%s", EN_SPI_3WIRE_MODE, val, val ? "4-wire" : "3-wire");
#ifdef ENABLE_3WIRE_SPI
    if ((val != 0xDEADBEEF) && (val & (1 << 0))) {
        val &= ~(1 << 0);
        SPI_DEBUG_SLAVE_TRACE(0, "===spi debug slave: 4wire spi switches to 3wire");
        spi_debug_slave_write_word_special(EN_SPI_3WIRE_MODE, val, spi_cfg.dma_tx);
        SPI_DEBUG_SLAVE_TRACE(0, "  ");
        while (1) {
            SPI_DEBUG_SLAVE_TRACE(0, "***Now cross the DI and DO lines, restart the host, do not restart the device......");
            SPI_DEBUG_SLAVE_DELAY_MS(2000);
        }
    } else {
        //*
        SPI_DEBUG_SLAVE_TRACE(0, "  ");
        SPI_DEBUG_SLAVE_TRACE(0, "===iomux: set miso pin function as gpio");
        //Step 1: Modify the function of gpio
        const struct HAL_IOMUX_PIN_FUNCTION_MAP miso_gpio[] = {
            {REMOVED_SPI_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
        };
        hal_iomux_init(miso_gpio, ARRAY_SIZE(miso_gpio));
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)REMOVED_SPI_PIN, HAL_GPIO_DIR_OUT, 1);

        //Step 2: Modify registers (required for some chips)
        #define HW_WRITE_REG(_Reg,_Value)       (*((volatile uint32_t*)(_Reg)) = (uint32_t)(_Value))
        #define HW_READ_REG(_Reg)               (*((volatile uint32_t*)(_Reg)))
        #define SPI_SPECIAL_REG_ADDR            (0x40086000 + 0x0AC)//1306
        #define SPI_SPECIAL_REG_BIT             (1 << 0)            //1306

        val = HW_READ_REG(SPI_SPECIAL_REG_ADDR);
        val |= SPI_SPECIAL_REG_BIT;
        HW_WRITE_REG(SPI_SPECIAL_REG_ADDR, val);

        val = 1;
        while (val) {
            spi_debug_slave_read_word_special(EN_SPI_3WIRE_MODE, &val, spi_cfg.dma_rx);
            SPI_DEBUG_SLAVE_TRACE(2, "---read 3wire spi reg[0x%X]:0x%X->%s", EN_SPI_3WIRE_MODE, val, val ? "4-wire" : "3-wire");
        }
    }
    SPI_DEBUG_SLAVE_TRACE(0, "  ");
#endif

#if !defined(CHIP_BEST1303)
    spi_debug_slave_write_word_special(EN_SPI_CTNS_READ, 0x1, spi_cfg.dma_tx);
    SPI_DEBUG_SLAVE_TRACE(1, "---enable continuous read[0x%X]  :write 1", EN_SPI_CTNS_READ);
#endif
    spi_debug_slave_read_word_special(EN_SPI_CTNS_READ, &val, spi_cfg.dma_rx);
    SPI_DEBUG_SLAVE_TRACE(2, "---read continuous read reg[0x%X]:%d->%s", EN_SPI_CTNS_READ, val, val ? "continuous mode" : "single mode");

    SPI_DEBUG_SLAVE_TRACE(0, "  ");
    SPI_DEBUG_SLAVE_TRACE(2, "---test start addr:0x%X,size:%d", TEST_RAM_ADDR, BUF_SIZE - CRC_SIZE);
    SPI_DEBUG_SLAVE_TRACE(0, "  ");
    while (1) {
        cnt++;

        SPI_DEBUG_SLAVE_DELAY_MS(100);//Waiting for the slave startup and reception processing
        rand_generator(buf, BUF_SIZE - CRC_SIZE);
        tx_crc = crc32_c(0, (const uint8_t *)buf, BUF_SIZE - CRC_SIZE);
        memcpy(buf + BUF_SIZE - CRC_SIZE, &tx_crc, CRC_SIZE);
#ifndef ENABLE_CONTINUOUS_TEST_MODE
        SPI_DEBUG_SLAVE_DUMP8("%02X ", buf, 32);
        //SPI_DEBUG_SLAVE_DUMP8("%02X ",buf+BUF_SIZE-32,32);
#endif
        ret = spi_debug_slave_write(TEST_RAM_ADDR, buf, BUF_SIZE, spi_cfg.dma_tx);
        SPI_DEBUG_SLAVE_TRACE(4, "---crc32:0x%08X,tx size:%d,ret:%d,cnt:%d", tx_crc, BUF_SIZE - CRC_SIZE, ret, cnt);

        memset(buf, 0, sizeof(buf));
#if defined(CHIP_BEST1501) || defined(CHIP_BEST2002)
        ret = spi_debug_slave_read_continuous(TEST_RAM_ADDR, buf, BUF_SIZE, spi_cfg.dma_rx);
#else
        ret = spi_debug_slave_read_non_continuous(TEST_RAM_ADDR, buf, BUF_SIZE, spi_cfg.dma_rx);
#endif
#ifndef ENABLE_CONTINUOUS_TEST_MODE
        SPI_DEBUG_SLAVE_DUMP8("%02X ", buf, 32);
        //SPI_DEBUG_SLAVE_DUMP8("%02X ",buf+BUF_SIZE-32,32);
#endif
        tx_crc = crc32_c(0, (const uint8_t *)buf, BUF_SIZE - CRC_SIZE);
        rx_crc = *((uint32_t *)(buf + BUF_SIZE - CRC_SIZE));
        if (rx_crc == tx_crc) {
            SPI_DEBUG_SLAVE_TRACE(5, "---[rx success]crc32:0x%08X,rx size:%d,ret:%d,error_cnt:%d,cnt:%d",
                                  rx_crc, BUF_SIZE - CRC_SIZE, ret, error_cnt, cnt);
        } else {
            error_cnt++;
            SPI_DEBUG_SLAVE_TRACE(6, "***[rx fail]calc crc32:0x%08X,rx crc32:0x%08X,rx size:%d,ret:%d,error_cnt:%d,cnt:%d",
                                  tx_crc, rx_crc, BUF_SIZE - CRC_SIZE, ret, error_cnt, cnt);
        }
#ifndef ENABLE_CONTINUOUS_TEST_MODE
        SPI_DEBUG_SLAVE_TRACE(0, "  ");
        SPI_DEBUG_SLAVE_TRACE(0, "The program is over, if you want to run it again, please press the reset button");
        break;
#endif
    }
#endif
}
#endif

