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
#ifdef UART_TEST

#include <string.h>
#include <stdlib.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_uart.h"
#include "hal_sysfreq.h"
#include "crc32_c.h"

#define     UART_TX_ENABLE                  //The default is tx, otherwise it is rx
//#define     UNFIXED_LEN_RX_TEST
//#define     UART_LOOPBACK_MODE
//#define     UART_MASS_DATA_TEST_MODE
//#define     UART_ENABLE_DMA               //dma is not used by default

#define     UART_PORT_ID    1               //The ID is used for testing, not for trace
#define     UART_BANDRATE   (10*1000*1000)  //The bandrate is used for testing, not for trace

#if defined(UART_MASS_DATA_TEST_MODE) && defined(UART_ENABLE_DMA)
    #error "Mass data mode does not support dma (dma can send a maximum of 4095 bytes each time),20210723"
#endif

#if 1
    #define UART_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
    #define UART_DUMP8(n, s, ...)    DUMP8(n, s, ##__VA_ARGS__)
    #define UART_DUMP16(n, s, ...)   DUMP16(n, s, ##__VA_ARGS__)
#else
    #define UART_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define UART_DUMP8(n, s, ...)
    #define UART_DUMP16(n, s, ...)
#endif

#if 1
    #define UART_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define UART_DELAY_US(us)          hal_sys_timer_delay_us(us)
#else
    #define UART_DELAY_MS(ms)
    #define UART_DELAY_US(us)
#endif

POSSIBLY_UNUSED static int32_t uart_write(uint8_t uart_id, uint8_t *buf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint32_t sent = 0;

    if (enable_dma) {
        ret = hal_uart_dma_send(uart_id, buf, len, NULL, NULL);
    } else {
        while (sent < len) {
            hal_uart_blocked_putc((enum HAL_UART_ID_T)uart_id, buf[sent++]);
        }
        if (sent != len) {
            ret = -2021;
        }
        ret = 0;
    }

    return ret;
}

POSSIBLY_UNUSED static int32_t uart_read(uint8_t uart_id, uint8_t *buf, uint32_t len, uint8_t enable_dma)
{
    int32_t ret = 0;
    uint32_t recv = 0;

    if (enable_dma) {
        ret = hal_uart_dma_recv(uart_id, buf, len, NULL, NULL);
    } else {
        while (recv < len) {
            buf[recv++] = hal_uart_blocked_getc((enum HAL_UART_ID_T)uart_id);
        }
        if (recv != len) {
            ret = -2021;
        }
        ret = 0;
    }

    return ret;
}

#if defined(UART_ENABLE_DMA) || defined(UNFIXED_LEN_RX_TEST)
static volatile uint8_t uart_rx_done = 0;
static volatile uint8_t uart_tx_done = 0;
static volatile uint32_t uart_rx_len = 0;
POSSIBLY_UNUSED static void uart_rx(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    if (dma_error) {
        UART_TRACE(1, "uart rx dma error: xfer_size=%d", xfer_size);
        uart_rx_len = 0;
    } else if (status.BE || status.FE || status.OE || status.PE) {
        UART_TRACE(2, "uart rx uart error: xfer_size=%d, status=0x%08x", xfer_size, status.reg);
        uart_rx_len = 0;
    } else {
        uart_rx_len = xfer_size;
    }
    uart_rx_done = 1;
}

POSSIBLY_UNUSED static void uart_tx(uint32_t xfer_size, int dma_error)
{
    if (dma_error) {
        UART_TRACE(1, "uart tx dma error: xfer_size=%d", xfer_size);
    }
    uart_tx_done = 1;
}
#endif

#ifdef UART_MASS_DATA_TEST_MODE

#define CRC_SIZE    4
#define BUF_SIZE    (32768+CRC_SIZE)
POSSIBLY_UNUSED static uint8_t buf[BUF_SIZE];

POSSIBLY_UNUSED static void rand_generator(uint8_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint8_t)rand();
    }
}

#else

#define RX_BUF_SIZE 1024    //When using dma, the maximum value is 4095
#if (RX_BUF_SIZE > 4095) && (defined(UART_ENABLE_DMA) || defined(UNFIXED_LEN_RX_TEST))
    #error "The maximum value of RX_BUF_SIZE cannot exceed 4095"
#endif

POSSIBLY_UNUSED static const uint8_t tx_buf[] = {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
POSSIBLY_UNUSED static uint8_t rx_buf[RX_BUF_SIZE];
#endif  /* UART_MASS_DATA_TEST_MODE */

void uart_test(void)
{
    int32_t ret;
    uint32_t cnt = 0;
    struct HAL_UART_CFG_T uart_cfg;
    POSSIBLY_UNUSED uint32_t error_cnt = 0;
#ifdef UART_MASS_DATA_TEST_MODE
    uint32_t crc = 0;
#endif

    UART_TRACE(0, "   ");
#if defined(CHIP_BEST1501)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
    while (1) {
        UART_TRACE(0, "***1501 chip does not support this test example temporarily");
        UART_DELAY_MS(1000);
    }
#elif defined(CHIP_BEST2003)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_390M);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
#endif
    UART_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    UART_TRACE(0, "------UART tx/rx test started------");

    memset(&uart_cfg, 0, sizeof(struct HAL_UART_CFG_T));
    uart_cfg.parity = HAL_UART_PARITY_NONE,
    uart_cfg.stop = HAL_UART_STOP_BITS_1,
    uart_cfg.data = HAL_UART_DATA_BITS_8,
    uart_cfg.flow = HAL_UART_FLOW_CONTROL_NONE,
    uart_cfg.tx_level = HAL_UART_FIFO_LEVEL_1_2,
    uart_cfg.rx_level = HAL_UART_FIFO_LEVEL_1_4,
    uart_cfg.baud = UART_BANDRATE,
    uart_cfg.dma_rx_stop_on_err = false;
#if defined(UART_ENABLE_DMA) || defined(UNFIXED_LEN_RX_TEST)
    uart_cfg.dma_rx = true,
    uart_cfg.dma_tx = true,
#else
    uart_cfg.dma_rx = false,
    uart_cfg.dma_tx = false,
#endif

#if   (UART_PORT_ID == 0)
    hal_iomux_set_uart0();
#elif (UART_PORT_ID == 1)
    hal_iomux_set_uart1();
#elif (UART_PORT_ID == 2)
    hal_iomux_set_uart2();
#elif (UART_PORT_ID == 3)
    hal_iomux_set_uart3();
#elif (UART_PORT_ID == 4)
    hal_iomux_set_uart4();
#elif (UART_PORT_ID == 5)
    hal_iomux_set_uart5();
#elif (UART_PORT_ID == 6)
    hal_iomux_set_uart6();
#elif (UART_PORT_ID == 7)
    hal_iomux_set_uart7();
#elif (UART_PORT_ID == 8)
    hal_iomux_set_uart8();
#elif (UART_PORT_ID == 9)
    hal_iomux_set_uart9();
#elif (UART_PORT_ID == 10)
    hal_iomux_set_uart10();
#else
#error "unsupported uart id"
#endif
    hal_uart_open((enum HAL_UART_ID_T)UART_PORT_ID, &uart_cfg);

#if defined(UART_ENABLE_DMA) || defined(UNFIXED_LEN_RX_TEST)
    hal_uart_irq_set_dma_handler((enum HAL_UART_ID_T)UART_PORT_ID, uart_rx, uart_tx);
#endif

    UART_TRACE(0, "  ");
    UART_TRACE(1, "uart_cfg.parity              :%d", uart_cfg.parity);
    UART_TRACE(1, "uart_cfg.stop                :%d", uart_cfg.stop);
    UART_TRACE(1, "uart_cfg.data                :%d", uart_cfg.data);
    UART_TRACE(1, "uart_cfg.flow                :%d", uart_cfg.flow);
    UART_TRACE(1, "uart_cfg.tx_level            :%d", uart_cfg.tx_level);
    UART_TRACE(1, "uart_cfg.rx_level            :%d", uart_cfg.rx_level);
    UART_TRACE(1, "uart_cfg.baud                :%d", uart_cfg.baud);
    UART_TRACE(1, "uart_cfg.dma_rx              :%d", uart_cfg.dma_rx);
    UART_TRACE(1, "uart_cfg.dma_tx              :%d", uart_cfg.dma_tx);
    UART_TRACE(1, "uart_cfg.dma_rx_stop_on_err  :%d", uart_cfg.dma_rx_stop_on_err);
    UART_TRACE(0, "  ");
    UART_TRACE(0, "****************************************");
    UART_TRACE(0, "*                                      *");
    UART_TRACE(0, "*    uart tx/rx test serial port:%d     *", UART_PORT_ID);
    UART_TRACE(0, "*                                      *");
    UART_TRACE(0, "****************************************");
#ifdef UART_TX_ENABLE
#if defined(UART_MASS_DATA_TEST_MODE)
    UART_TRACE(0, "*          tx mass data mode           *");
#elif defined(UART_LOOPBACK_MODE)
    UART_TRACE(0, "*             loopback mode            *");
#elif defined(UNFIXED_LEN_RX_TEST)
    UART_TRACE(0, "*        unfixed length rx mode        *");
#else
    UART_TRACE(0, "*            tx simple mode            *");
#endif
#else
#if defined(UART_MASS_DATA_TEST_MODE)
    UART_TRACE(0, "*          rx mass data mode           *");
#elif defined(UART_LOOPBACK_MODE)
    UART_TRACE(0, "*             loopback mode            *");
#elif defined(UNFIXED_LEN_RX_TEST)
    UART_TRACE(0, "*        unfixed length rx mode        *");
#else
    UART_TRACE(0, "*            rx simple mode            *");
#endif
#endif
    UART_TRACE(0, "****************************************");
    UART_TRACE(0, "  ");

#ifdef UART_MASS_DATA_TEST_MODE
    srand(hal_sys_timer_get());
#endif

#ifdef UNFIXED_LEN_RX_TEST
    union HAL_UART_IRQ_T mask;
    mask.reg = 0;   //Clear all interruptions
    mask.RT = 1;    //1: Open the interrupt, 0 closed interrupt
    mask.FE = 1;
    mask.PE = 0;
    mask.BE = 1;
    mask.OE = 1;

    while (1) {
        UART_TRACE(0, "  ");
        UART_TRACE(0, "Please enter up to 4095 characters on the serial port %d...", UART_PORT_ID);
        ret = hal_uart_dma_recv_mask((enum HAL_UART_ID_T)UART_PORT_ID, rx_buf, RX_BUF_SIZE, NULL, NULL, &mask);
        while (!uart_rx_done);
        uart_rx_done = 0;

        UART_TRACE(0, "ret:%d,rx len:%d,rx data:%s", ret, uart_rx_len, rx_buf);
        ret = hal_uart_dma_send((enum HAL_UART_ID_T)UART_PORT_ID, (uint8_t *)rx_buf, uart_rx_len, NULL, NULL);
        while (!uart_tx_done);
        uart_tx_done = 0;

        memset(rx_buf, 0, sizeof(rx_buf));
        UART_TRACE(0, "Serial port %d receiving and transmits data completed,ret:%d", UART_PORT_ID, ret);
    }
#endif /* UNFIXED_LEN_RX_TEST */

    while (1) {
        cnt++;
        hal_cmu_simu_fail();
#if defined(UART_MASS_DATA_TEST_MODE)
#ifdef UART_TX_ENABLE
        UART_DELAY_MS(50);//Waiting for the slave startup and reception processing
        rand_generator(buf, BUF_SIZE - CRC_SIZE);
        crc = crc32_c(0, (const uint8_t *)buf, BUF_SIZE - CRC_SIZE);
        //UART_DUMP8("%02X ",buf,16);
        memcpy(buf + BUF_SIZE - CRC_SIZE, &crc, CRC_SIZE);
        ret = uart_write((enum HAL_UART_ID_T)UART_PORT_ID, buf, BUF_SIZE, uart_cfg.dma_tx);
        UART_TRACE(4, "---crc32:0x%08X,tx size:%d,ret:%d,cnt:%d", crc, BUF_SIZE, ret, cnt);
#else
        uint32_t rx_crc;

        ret = uart_read((enum HAL_UART_ID_T)UART_PORT_ID, buf, BUF_SIZE, uart_cfg.dma_rx);
        crc = crc32_c(0, (const uint8_t *)buf, BUF_SIZE - CRC_SIZE);
        rx_crc = *((uint32_t *)(buf + BUF_SIZE - CRC_SIZE));
        if (rx_crc == crc) {
            UART_TRACE(5, "---[rx success]crc32:0x%08X,rx size:%d,ret:%d,error_cnt:%d,cnt:%d", crc, BUF_SIZE, ret, error_cnt, cnt);
        } else {
            error_cnt++;
            UART_TRACE(6, "***[rx fail]calc crc32:0x%08X,rx crc32:0x%08X,rx size:%d,ret:%d,error_cnt:%d,cnt:%d", crc, rx_crc, BUF_SIZE, ret, error_cnt, cnt);
        }
        memset(buf, 0, sizeof(buf));
#endif

#elif defined(UART_LOOPBACK_MODE)
#define INPUT_LEN 10

        UART_TRACE(0, "  ");
        UART_TRACE(0, "Please enter %d characters on the serial port %d...", INPUT_LEN, UART_PORT_ID);
        ret = uart_read((enum HAL_UART_ID_T)UART_PORT_ID, rx_buf, INPUT_LEN, uart_cfg.dma_rx);
#ifdef UART_ENABLE_DMA
        while (!uart_rx_done);
        uart_rx_done = 0;
#endif
        UART_TRACE(0, "ret:%d,rx len:%d,rx data:%s", ret, uart_rx_len, rx_buf);
        ret = uart_write((enum HAL_UART_ID_T)UART_PORT_ID, (uint8_t *)rx_buf, INPUT_LEN, uart_cfg.dma_tx);
#ifdef UART_ENABLE_DMA
        while (!uart_tx_done);
        uart_tx_done = 0;
#endif
        memset(rx_buf, 0, sizeof(rx_buf));
        UART_TRACE(0, "Serial port %d transmits and receiving data completed,ret:%d", UART_PORT_ID, ret);
        UART_DELAY_MS(50);
#else

#ifdef UART_TX_ENABLE
        UART_DELAY_MS(50);
        ret = uart_write((enum HAL_UART_ID_T)UART_PORT_ID, (uint8_t *)tx_buf, sizeof(tx_buf), uart_cfg.dma_tx);
#ifdef UART_ENABLE_DMA
        while (!uart_tx_done);
        uart_tx_done = 0;
#endif
        UART_TRACE(3, "tx:%s,ret:%d,cnt:%d", tx_buf, ret, cnt);
#else
        ret = uart_read((enum HAL_UART_ID_T)UART_PORT_ID, rx_buf, sizeof(tx_buf), uart_cfg.dma_rx);
#ifdef UART_ENABLE_DMA
        while (!uart_rx_done);
        uart_rx_done = 0;
#endif
        ret = memcmp(rx_buf, tx_buf, sizeof(tx_buf));
        if (ret) {
            error_cnt++;
        }
        UART_TRACE(3, "rx:%s,ret:%d,cnt:%d,error_cnt:%d", rx_buf, ret, cnt, error_cnt);
        memset(rx_buf, 0, sizeof(rx_buf));
#endif  //UART_TX_ENABLE
#endif  //UART_MASS_DATA_TEST_MODE
        hal_cmu_simu_pass();
    }
}

#endif

