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
#ifdef SDIO_HOST_TEST

#include "hal_sdio_host.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_dma.h"
#include <string.h>
#include "pmu.h"

#if 1
    #define SDIO_HOST_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define SDIO_HOST_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
    #define SDIO_HOST_DUMP16(n, s, ...)     DUMP16(n, s, ##__VA_ARGS__)
#else
    #define SDIO_HOST_TRACE(n, s, ...)      TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SDIO_HOST_DUMP8(n, s, ...)
    #define SDIO_HOST_DUMP16(n, s, ...)
#endif

#if 1
    #define SDIO_HOST_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define SDIO_HOST_DELAY_US(us)          hal_sys_timer_delay_us(us)
#else
    #define SDIO_HOST_DELAY_MS(ms)
    #define SDIO_HOST_DELAY_US(us)
#endif

#define SDIO_HOST_ID    HAL_SDIO_HOST_ID_0

//Macros that may need to be turned on or off
#define SDIO_HOST_TX_RX_USE_DMA
//#define SDIO_HOST_POLLING_MODE
//#define ENABLE_WIFI_TXRX_TEST

//#define BOARD_POWER_ON
//#define POWER_DOWN_SLEEP_TEST
//#define SDIO_DEVICE_RE_ENUM_TEST    //SDIO device repeated enumeration test
//#define ENABLE_DEVICE_SDIO_WAKEUP_TEST
//#define ENABLE_DEVICE_GPIO_WAKEUP_TEST
//#define ENABLE_SDIO_READ_REG_TEST

#if defined(ENABLE_DEVICE_SDIO_WAKEUP_TEST) && defined(ENABLE_DEVICE_GPIO_WAKEUP_TEST)
    #error "SDIO and GPIO wakeup cannot be performed at the same time"
#endif

#if defined(ENABLE_DEVICE_SDIO_WAKEUP_TEST) && defined(ENABLE_DEVICE_GPIO_WAKEUP_TEST) && defined(ENABLE_SDIO_READ_REG_TEST)
    #error "SDIO/GPIO wakeup and READ_REG test cannot be done at the same time"
#endif

#define MAX_DUMP_LEN    16
#define BUFF_LEN        32768
static uint8_t  data_buff[BUFF_LEN];

static volatile uint8_t enable_read_device = 0;
static uint16_t real_read_len = 0;
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
    static volatile uint8_t dma_txrx_done = 0;
#endif

#ifdef ENABLE_SDIO_READ_REG_TEST
static uint8_t crc8_itu(uint8_t *data, uint32_t length)
{
    uint8_t i;
    uint8_t crc = 0;    // Initial value
    while (length--) {
        crc ^= *data++; // crc ^= *data; data++;
        for (i = 0; i < 8; i++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc ^ 0x55;
}
#endif

static void hal_sdio_host_card_detect(void)
{
    SDIO_HOST_TRACE(0, "------sdio host detected a card");
}

static void hal_sdio_host_signal_voltage_switch(void)
{
#if defined(CHIP_BEST2002) || defined(CHIP_BEST2005)
    hal_iomux_set_sdio_voltage(HAL_IOMUX_PIN_VOLTAGE_MEM);
#endif

    //Complete signal voltage switching within 5ms,2021-07-04
    SDIO_HOST_TRACE(0, "------sdio host signal voltage switching completed");
}

static void hal_sdio_host_error(enum HAL_SDIO_HOST_ERR error)
{
    SDIO_HOST_TRACE(1, "******sdio host error:%d", error);
}

static void hal_sdio_host_func_int(enum HAL_SDIO_FUNC_NUM_T func_num)
{
    SDIO_HOST_TRACE(1, "------host has received device func%d int", func_num);
    uint8_t int_data_lsb = 0;
    uint8_t int_data_hsb = 0;
    #define SDIO_FUNC1_INT_VALUE1       0x0
    #define SDIO_FUNC1_INT_VALUE2       0x1

    hal_sdio_io_rw_direct(SDIO_HOST_ID, HAL_SDIO_DEVICE_READ, func_num, SDIO_FUNC1_INT_VALUE1, 0, &int_data_lsb);
    hal_sdio_io_rw_direct(SDIO_HOST_ID, HAL_SDIO_DEVICE_READ, func_num, SDIO_FUNC1_INT_VALUE2, 0, &int_data_hsb);

    enable_read_device = 1;
    real_read_len = ((uint16_t)(int_data_hsb << 8)) + int_data_lsb;
}

#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
static void hal_sdio_host_dma_tx_done(uint32_t remain_size, uint32_t error)
{
    SDIO_HOST_TRACE(0, "------host tx done");

    dma_txrx_done = 1;
}

static void hal_sdio_host_dma_rx_done(uint32_t remain_size, uint32_t error)
{
    SDIO_HOST_TRACE(0, "------host rx done");

    //hal_dma_sync_for_cpu(data_buff, sizeof(data_buff));
    dma_txrx_done = 1;
}
#endif

static struct HAL_SDIO_HOST_CB_T sdio_host_test_callback = {
    hal_sdio_host_card_detect,
    hal_sdio_host_signal_voltage_switch,
    hal_sdio_host_error,
    hal_sdio_host_func_int,
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
    hal_sdio_host_dma_tx_done,
    hal_sdio_host_dma_rx_done,
#else
    NULL,
    NULL,
#endif
};

void sdio_host_test(void)
{
    uint32_t i;
    uint8_t  response_r5;
    uint8_t  buf_cnt = 0;
    uint8_t  capability;
    uint8_t  bus_speed_mode;
    uint16_t device_rx_buf_len_max;
    uint16_t real_write_len = 0;
    uint16_t aligned_write_len = 0;
    uint16_t blocksize;
    uint16_t *write_p = (uint16_t *)data_buff;
    struct HAL_SDIO_HOST_CONFIG_T sdio_cfg;
    enum HAL_SDIO_ERR err;

#ifdef BOARD_POWER_ON
#define BOARD_PWR_EN        HAL_GPIO_PIN_P8_4
#define BOARD_PWR_ON        HAL_GPIO_PIN_P8_5
#define BOARD_PWR_RESET     HAL_GPIO_PIN_P9_3
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {BOARD_PWR_EN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {BOARD_PWR_ON, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {BOARD_PWR_RESET, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(BOARD_PWR_EN, HAL_GPIO_DIR_OUT, 1);//output high
    hal_gpio_pin_set_dir(BOARD_PWR_ON, HAL_GPIO_DIR_OUT, 1);//output high
    SDIO_HOST_DELAY_MS(500);
    hal_gpio_pin_set_dir(BOARD_PWR_RESET, HAL_GPIO_DIR_OUT, 1);//output high
    SDIO_HOST_DELAY_MS(200);
    hal_gpio_pin_clr(BOARD_PWR_RESET);  //reset complete
    SDIO_HOST_DELAY_MS(1000);           //wait device init done
    SDIO_HOST_TRACE(0, "------sdio device has been turned on------");
#endif

    SDIO_HOST_TRACE(0, "   ");
    SDIO_HOST_TRACE(0, "------sdio host test started------");
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
    SDIO_HOST_TRACE(0, "------sdio host runs in dma transceiver non-blocking/callback mode");
#elif defined(SDIO_HOST_TX_RX_USE_DMA) && defined(SDIO_HOST_POLLING_MODE)
    SDIO_HOST_TRACE(0, "------sdio host runs in dma transceiver blocking mode");
#else
    SDIO_HOST_TRACE(0, "------sdio host runs in non-dma transceiver blocking mode");
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    SDIO_HOST_TRACE(0, "   ");
    SDIO_HOST_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SDIO_HOST_DELAY_MS(5);//For palladium and chip simulation:wait device run first,2021-05-15

    memset(&sdio_cfg, 0, sizeof(sdio_cfg));
    sdio_cfg.yield = 0;
#ifdef SDIO_HOST_TX_RX_USE_DMA
    sdio_cfg.dma_en = 1;
#else
    sdio_cfg.dma_en = 0;
#endif
    sdio_cfg.ddr_mode = 0;//ip does not support
    sdio_cfg.func_en = 1;
    sdio_cfg.send_abort = 1;
    sdio_cfg.func1_only = 1;
    sdio_cfg.async_int_en = 1;
    sdio_cfg.set_func_blks = 1;
    sdio_cfg.volt_switch = 1;
    sdio_cfg.blocking_init_en = 1;
    sdio_cfg.block_size = 256;
    sdio_cfg.bus_speed = 48 * 1000 * 1000;
    sdio_cfg.bus_width = 4;
    sdio_cfg.callback = &sdio_host_test_callback;

    err = hal_sdio_host_open(SDIO_HOST_ID, &sdio_cfg);
    if (err) {
        SDIO_HOST_TRACE(0, "******hal_sdio_host_open error:%d", err);
        return;
    }
    hal_sdio_get_block_size(SDIO_HOST_ID, HAL_SDIO_FUNC_1, &blocksize);
    SDIO_HOST_TRACE(1, "------sdio block size:%d", blocksize);
    hal_sdio_get_capability(SDIO_HOST_ID, &capability);
    SDIO_HOST_TRACE(1, "------sdio device capability:0x%X", capability);
    hal_sdio_get_bus_speed(SDIO_HOST_ID, &bus_speed_mode);
    SDIO_HOST_TRACE(1, "------sdio speed mode:%d", bus_speed_mode);
    SDIO_HOST_TRACE(1, "------sdio bus speed:%d", hal_sdio_get_host_speed(SDIO_HOST_ID));
    SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, " ");
    SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, " ");
    SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, " ");

#ifdef POWER_DOWN_SLEEP_TEST
    TRACE_FLUSH();
    return;
#endif

#ifdef ENABLE_DEVICE_SDIO_WAKEUP_TEST
    uint32_t send_retry_cnt = 0;
    uint32_t send_wakeup_cnt = 1;
    SDIO_HOST_TRACE(0, "------The host sends an interrupt to the device to put it to sleep");
    hal_sdio_host_gen_int_to_device(SDIO_HOST_ID);
    SDIO_HOST_DELAY_MS(2000);
    SDIO_HOST_TRACE(0, "------wait for device to sleep");

    while (1) {
        send_wakeup_cnt++;
        if (send_wakeup_cnt >= 8192) {
            send_wakeup_cnt = 1;
        }

wait_device_wakeup:
        SDIO_HOST_TRACE(0, "------The host sends an interrupt to wakeup device, wakeup cnt:%d, retry cnt:%d", send_wakeup_cnt, send_retry_cnt);
        enable_read_device = 0;
        hal_sdio_host_gen_int_to_device(SDIO_HOST_ID);
        SDIO_HOST_TRACE(0, "------wait device wakeup");
        i = 0;
        while (!enable_read_device) {
            SDIO_HOST_DELAY_MS(1);

            i++;
            if (i >= 2000) {  //2s timeout
                send_retry_cnt++;
                goto wait_device_wakeup;
            }
        }
        SDIO_HOST_TRACE(1, "------sdio device wakeup cnt                                  :%d", real_read_len);
        SDIO_HOST_TRACE(0, "  ");

        if (send_wakeup_cnt != real_read_len) {
            SDIO_HOST_TRACE(0, "******The number of wakeups of the host is different from that of the device");
            SDIO_HOST_TRACE(0, "------Enter while1, please analyze the reason");
            while (1);
        }
        SDIO_HOST_DELAY_MS(1000);
    }
#endif

#ifdef ENABLE_DEVICE_GPIO_WAKEUP_TEST
    #define WAKEUP_GPIO HAL_GPIO_PIN_P2_7

    uint32_t send_wakeup_cnt = 0;
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {WAKEUP_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(WAKEUP_GPIO, HAL_GPIO_DIR_OUT, 0);

    while (1) {
        send_wakeup_cnt++;
        hal_gpio_pin_set(WAKEUP_GPIO);
        SDIO_HOST_TRACE(0, "------The host sends an GPIO toggle to wakeup device, val:1, wakeup cnt:%d", send_wakeup_cnt);
        SDIO_HOST_DELAY_MS(500);

        send_wakeup_cnt++;
        hal_gpio_pin_clr(WAKEUP_GPIO);
        SDIO_HOST_TRACE(0, "------The host sends an GPIO toggle to wakeup device, val:0, wakeup cnt:%d", send_wakeup_cnt);
        SDIO_HOST_DELAY_MS(500);
    }
#endif

#ifdef ENABLE_SDIO_READ_REG_TEST
    uint32_t send_retry_cnt = 0;
    uint32_t send_readreg_cnt = 0;
    uint32_t send_readreg_cnt_crc8 = 0;
    uint8_t readreg[4];
    uint8_t crc8;
    uint32_t readreg_val;
    #define READ_REG_ADDR 0x104
    #define READ_REG_MASK 0xFFFFFF

    while (1) {
        send_readreg_cnt++;
        if (send_readreg_cnt > READ_REG_MASK) {
            send_readreg_cnt = 1;
        }

        crc8 = crc8_itu((uint8_t *)&send_readreg_cnt, 3);
        send_readreg_cnt_crc8 = (crc8 << 24) | send_readreg_cnt;
wait_device_response:
        SDIO_HOST_TRACE(0, "------The host sends an interrupt to device, send cnt:(0x%02X)%d, retry cnt:%d", crc8, send_readreg_cnt, send_retry_cnt);
        enable_read_device = 0;
        hal_sdio_host_gen_int_to_device(SDIO_HOST_ID);
        SDIO_HOST_TRACE(0, "------wait device response");
        i = 0;
        while (!enable_read_device) {
            SDIO_HOST_DELAY_MS(1);

            i++;
            if (i >= 2000) {  //2s timeout
                send_retry_cnt++;
                goto wait_device_response;
            }
        }
        SDIO_HOST_TRACE(1, "------sdio device response cnt                       :      %d", real_read_len);

        hal_sdio_io_rw_direct(SDIO_HOST_ID, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, READ_REG_ADDR + 0, 0, &readreg[0]);
        hal_sdio_io_rw_direct(SDIO_HOST_ID, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, READ_REG_ADDR + 1, 0, &readreg[1]);
        hal_sdio_io_rw_direct(SDIO_HOST_ID, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, READ_REG_ADDR + 2, 0, &readreg[2]);
        hal_sdio_io_rw_direct(SDIO_HOST_ID, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, READ_REG_ADDR + 3, 0, &readreg[3]);
        readreg_val = *((uint32_t *)readreg);
        SDIO_HOST_TRACE(1, "------sdio host read reg                             :(0x%02X)%d, 0x%X", readreg[3], readreg_val & READ_REG_MASK, readreg_val & READ_REG_MASK);
        SDIO_HOST_TRACE(0, "  ");

        if (((send_readreg_cnt & 0xFFFF) != real_read_len) || (send_readreg_cnt_crc8 != readreg_val)) {
            SDIO_HOST_TRACE(0, "******The data between host and device does not match");
            SDIO_HOST_TRACE(0, "------Enter while1, please analyze the reason");
            while (1);
        }
        SDIO_HOST_DELAY_MS(5);
    }
#endif

    while (1) {
#ifdef ENABLE_WIFI_TXRX_TEST
        //---------------------------read test----------------------------------
        if (enable_read_device) {
            enable_read_device = 0;
            SDIO_HOST_TRACE(1, "------sdio host read real len:%d", real_read_len);
            ASSERT(real_read_len && (real_read_len <= BUFF_LEN), "%s:%d,sdio host get device tx buf len:%d", __func__, __LINE__, real_read_len);
            memset(data_buff, 0xAA, sizeof(data_buff));
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
            dma_txrx_done = 0;
#endif
            hal_sdio_io_rw_extended(SDIO_HOST_ID, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, 0,
                                    &response_r5, 1, (uint8_t *)data_buff, real_read_len);
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
            while (!dma_txrx_done) {
                SDIO_HOST_DELAY_US(10);
            }
            dma_txrx_done = 0;
#endif
            SDIO_HOST_TRACE(1, "------sdio cmd53 read complete------,rsp=0x%X", response_r5);
            SDIO_HOST_TRACE(0, "------dump data start------");
            write_p = (uint16_t *)data_buff;
            for (; write_p < (uint16_t *)(data_buff + real_read_len); write_p += MAX_DUMP_LEN) {
                SDIO_HOST_DUMP16("%04X ", write_p, MAX_DUMP_LEN);
            }
            SDIO_HOST_TRACE(0, "------dump data complete------");
        }
#else
        //---------------------------read test----------------------------------
wait_device_int:
        SDIO_HOST_TRACE(0, "------The host actively sends an interrupt to the device");
        hal_sdio_host_gen_int_to_device(SDIO_HOST_ID);
        //There is no need to post this from today onwards, because it is processed in the open function,2021-06-25
        //hal_sdio_set_func_abort(SDIO_HOST_ID, 1); //In order to get the asynchronous interrupt sent by the device
        SDIO_HOST_TRACE(0, "------wait device int");
        i = 0;
        while (!enable_read_device) {
            SDIO_HOST_DELAY_MS(1);

            i++;
            if (i >= 1000) {  //1s timeout
                goto wait_device_int;
            }
        }
        enable_read_device = 0;
        SDIO_HOST_TRACE(1, "------sdio host read real len:%d", real_read_len);
        ASSERT(real_read_len && (real_read_len <= BUFF_LEN), "%s:%d,sdio host get device tx buf len:%d", __func__, __LINE__, real_read_len);
        memset(data_buff, 0xAA, sizeof(data_buff));
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
        dma_txrx_done = 0;
#endif
        hal_sdio_io_rw_extended(SDIO_HOST_ID, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, 0,
                                &response_r5, 1, (uint8_t *)data_buff, real_read_len);
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
        while (!dma_txrx_done) {
            SDIO_HOST_DELAY_US(10);
        }
        dma_txrx_done = 0;
#endif
        SDIO_HOST_TRACE(1, "------sdio cmd53 read complete------,rsp=0x%X", response_r5);
        SDIO_HOST_TRACE(0, "------dump data start------");
        write_p = (uint16_t *)data_buff;
        for (; write_p < (uint16_t *)(data_buff + real_read_len); write_p += MAX_DUMP_LEN) {
            SDIO_HOST_DUMP16("%04X ", write_p, MAX_DUMP_LEN);
        }
        SDIO_HOST_TRACE(0, "------dump data complete------");
#endif

        //----------------------------write test--------------------------------
        //write
        write_p = (uint16_t *)data_buff;
        for (i = 0; i < BUFF_LEN / 2; i++) {
            write_p[i] = i;
        }

#ifndef ENABLE_WIFI_TXRX_TEST
wait_buf_cnt:
#endif
        if (!buf_cnt) {
            hal_sdio_host_get_func1_rx_buf_cfg(SDIO_HOST_ID, &buf_cnt, &device_rx_buf_len_max);
            SDIO_HOST_TRACE(2, "------sdio device rx buf cnt:%d,buf len max:%d", buf_cnt, device_rx_buf_len_max);
        }
        if (BUFF_LEN < device_rx_buf_len_max) {
            SDIO_HOST_TRACE(2, "%s:%d", __func__, __LINE__);
            ASSERT(0, "******sdio host buf is too short %d ,len at least %d", BUFF_LEN, device_rx_buf_len_max);
        }

        if (buf_cnt) {
            real_write_len += 2;
            aligned_write_len = (real_write_len + 3) & (~3);    //Round up to a multiple of 4 bytes
            SDIO_HOST_TRACE(1, "------sdio host tx data size:%d", real_write_len);
            //hal_dma_sync_for_device(data_buff, sizeof(data_buff));
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
            dma_txrx_done = 0;
#endif
            hal_sdio_io_rw_extended(SDIO_HOST_ID, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_1, real_write_len,
                                    &response_r5, 1, (uint8_t *)data_buff, aligned_write_len);
#if defined(SDIO_HOST_TX_RX_USE_DMA) && !defined(SDIO_HOST_POLLING_MODE)
            while (!dma_txrx_done) {
                SDIO_HOST_DELAY_US(10);
            }
            dma_txrx_done = 0;
#endif
            SDIO_HOST_TRACE(2, "------sdio cmd53 write complete---,rsp=0x%X,buf_cnt=%d", response_r5, buf_cnt);
            buf_cnt--;
        } else {
            SDIO_HOST_TRACE(1, "******sdio device rx buf cnt:%d", buf_cnt);
#ifndef ENABLE_WIFI_TXRX_TEST
            SDIO_HOST_DELAY_MS(100);
            goto wait_buf_cnt;
#endif
        }

        if (real_write_len >= device_rx_buf_len_max) {
            real_write_len = 0;
        }

#ifndef ENABLE_WIFI_TXRX_TEST
        SDIO_HOST_DELAY_MS(100);//wait for write complete
#endif

#ifdef SDIO_DEVICE_RE_ENUM_TEST
        TRACE_FLUSH();
        SDIO_HOST_TRACE(0, "  ");
        SDIO_HOST_TRACE(0, "------reset sdio device");
        hal_sdio_reset(SDIO_HOST_ID);
        SDIO_HOST_TRACE(0, "------wait device re-init done");
        SDIO_HOST_DELAY_MS(500);
        pmu_reboot();
#endif
    }
}

#endif

