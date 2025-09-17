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
#ifdef SDIO_DEVICE_TEST

#include "hal_sdio_device.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include <string.h>

#include "plat_addr_map.h"
#ifdef __MCU_FW_2002__
    #include "hal_transq.h"
    #include "wifi_drv.h"
    #include "wifi_test.h"
#endif

#if 1
    #define SDIO_DEVICE_TRACE(n, s, ...)      TRACE(n, s, ##__VA_ARGS__)
    #define SDIO_DEVICE_DUMP8(n, s, ...)      DUMP8(n, s, ##__VA_ARGS__)
    #define SDIO_DEVICE_DUMP16(n, s, ...)     DUMP16(n, s, ##__VA_ARGS__)
#else
    #define SDIO_DEVICE_TRACE(n, s, ...)      TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SDIO_DEVICE_DUMP8(n, s, ...)
    #define SDIO_DEVICE_DUMP16(n, s, ...)
#endif

#if 1
    #define SDIO_DEVICE_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define SDIO_DEVICE_DELAY_US(us)          hal_sys_timer_delay_us(us)
#else
    #define SDIO_DEVICE_DELAY_MS(ms)
    #define SDIO_DEVICE_DELAY_US(us)
#endif

/* After opening this macro, the example can only be loaded from rom */
//#define ENABLE_APP_MODE

//#define ENABLE_TX_SELF_INCREASE
//#define ENABLE_SDIO_WAKEUP_TEST
//#define ENABLE_GPIO_WAKEUP_TEST
//#define ENABLE_SDIO_READ_REG_TEST

//#define ENABLE_PEAK_BW_RX_TEST
//#define ENABLE_PEAK_BW_TX_TEST
//#define PEAK_DATA_SIZE_MIN      1600

#define TXRX_BUFF_CNT       1
#define TXRX_BUFF_SIZE      8192    //16384
#define FORMATTED_DATA      0xAA

static const enum HAL_SDIO_ID_T sdio_id = HAL_SDIO_ID_0;

#if (TXRX_BUFF_SIZE % 4)
    #error "The buffer size must be an integer multiple of 4"
#endif

#if defined(ENABLE_SDIO_WAKEUP_TEST) && defined(ENABLE_GPIO_WAKEUP_TEST)
    #error "SDIO and GPIO wakeup cannot be performed at the same time"
#endif

#if defined(ENABLE_SDIO_WAKEUP_TEST) && defined(ENABLE_GPIO_WAKEUP_TEST) && defined(ENABLE_SDIO_READ_REG_TEST)
    #error "SDIO/GPIO wakeup and READ_REG test cannot be done at the same time"
#endif

#if defined(ENABLE_SDIO_WAKEUP_TEST) || defined(ENABLE_GPIO_WAKEUP_TEST) || defined(ENABLE_SDIO_READ_REG_TEST)
    volatile static uint32_t wakeup_cnt = 0;
#endif
static uint8_t __attribute__((aligned(4))) rx_buffer[TXRX_BUFF_CNT][TXRX_BUFF_SIZE];
static uint8_t __attribute__((aligned(4))) tx_buffer[TXRX_BUFF_SIZE];
static uint8_t rxbuf_index = 0;
static void hal_sdio_device_enum_done(enum HAL_SDIO_DEVICE_STATUS status)
{
    SDIO_DEVICE_TRACE(1, "------sdio device enumeration result:%d", status);

    //------Now the device can start communicating with the host------
}

static void hal_sdio_device_tx_start(uint32_t reg_addr)
{
#if !defined(ENABLE_PEAK_BW_RX_TEST) && !defined(ENABLE_PEAK_BW_TX_TEST)
    SDIO_DEVICE_TRACE(1, "------sdio device start tx");
#endif

#if defined(ENABLE_PEAK_BW_RX_TEST) || defined(ENABLE_PEAK_BW_TX_TEST)
    static uint8_t  once_flag = 0;
    static uint32_t time_cnt = 0;
    static uint32_t rxtx_cnt = 0;

    rxtx_cnt += reg_addr;
    if (!once_flag) {
        once_flag = 1;
        rxtx_cnt = 0;
        time_cnt = hal_fast_sys_timer_get();
    } else if (FAST_TICKS_TO_MS(hal_fast_sys_timer_get() - time_cnt) >= 1000) { //1000ms
        SDIO_DEVICE_TRACE(2, "------sdio device tx speed: %dB/s [%dKB/s]", rxtx_cnt, rxtx_cnt / 1024);
        once_flag = 0;
    }
#endif
}

static void hal_sdio_device_rx_start(uint32_t reg_addr)
{
#if !defined(ENABLE_PEAK_BW_RX_TEST) && !defined(ENABLE_PEAK_BW_TX_TEST)
    SDIO_DEVICE_TRACE(1, "------sdio device start rx");
#endif

#if defined(ENABLE_PEAK_BW_RX_TEST) || defined(ENABLE_PEAK_BW_TX_TEST)
    static uint8_t  once_flag = 0;
    static uint32_t time_cnt = 0;
    static uint32_t rxtx_cnt = 0;

    rxtx_cnt += reg_addr;
    if (!once_flag) {
        once_flag = 1;
        rxtx_cnt = 0;
        time_cnt = hal_fast_sys_timer_get();
    } else if (FAST_TICKS_TO_MS(hal_fast_sys_timer_get() - time_cnt) >= 1000) { //1000ms
        SDIO_DEVICE_TRACE(2, "------sdio device rx speed: %dB/s [%dKB/s]", rxtx_cnt, rxtx_cnt / 1024);
        once_flag = 0;
    }
#endif

    //Ready to receive the current packet of data
    hal_sdio_device_recv(sdio_id, &rx_buffer[rxbuf_index][0], TXRX_BUFF_SIZE, TXRX_BUFF_CNT);

    rxbuf_index++;
    if (rxbuf_index >= TXRX_BUFF_CNT) {
        rxbuf_index = 0;
    }

    if (TXRX_BUFF_CNT >= 2) {
        //clear the next buf
        memset(&rx_buffer[rxbuf_index][0], FORMATTED_DATA, TXRX_BUFF_SIZE);
    }
    if (TXRX_BUFF_CNT >= 3) {
        //Clear the next buf of the next
        uint8_t index_next;

        index_next = rxbuf_index;
        index_next++;
        if (index_next >= TXRX_BUFF_CNT) {
            index_next = 0;
        }
        memset(&rx_buffer[index_next][0], FORMATTED_DATA, TXRX_BUFF_SIZE);
    }
}

static void hal_sdio_device_send_done(const uint8_t *buf, uint32_t buf_len)
{
#ifndef ENABLE_PEAK_BW_TX_TEST
    SDIO_DEVICE_TRACE(2, "------sdio device send done, addr:0x%X,len:%d", (uint32_t)buf, buf_len);
#endif
}

#ifndef ENABLE_PEAK_BW_RX_TEST
static void buf_dump16(const uint8_t *buf, uint32_t len)
{
    #define MAX_DUMP_LEN 16
    uint16_t *dump_p = (uint16_t *)buf;

    SDIO_DEVICE_TRACE(4, "%s:%d,buf addr=0x%X,len=%d", __func__, __LINE__, (uint32_t)buf, len);
    SDIO_DEVICE_TRACE(0, "------dump data start------");
    for (; dump_p < (uint16_t *)(buf + len); dump_p += MAX_DUMP_LEN) {
        DUMP16("%04X ", dump_p, MAX_DUMP_LEN);
    }
    SDIO_DEVICE_TRACE(0, "------dump data complete------");
}
#endif

static void hal_sdio_device_recv_done(uint8_t *buf, uint32_t len)
{
    //------------------The host data received is processed here----------------
#ifndef ENABLE_PEAK_BW_RX_TEST
    buf_dump16(buf, len);
#endif

    //Ready to receive the next packet of data
    hal_sdio_device_recv(sdio_id, &rx_buffer[rxbuf_index][0], TXRX_BUFF_SIZE, TXRX_BUFF_CNT);
}

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

static void hal_sdio_device_int_from_host(void)
{
    enum HAL_SDIO_DEVICE_STATUS status;

#ifndef ENABLE_PEAK_BW_TX_TEST
    SDIO_DEVICE_TRACE(0, "------sdio device received an int form the host");
#ifdef ENABLE_SDIO_WAKEUP_TEST
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_208M);
    wakeup_cnt++;
    if (wakeup_cnt >= TXRX_BUFF_SIZE) {
        wakeup_cnt = 1;
    }
    status = hal_sdio_device_send(sdio_id, (const uint8_t *)tx_buffer, TXRX_BUFF_SIZE, wakeup_cnt);
    SDIO_DEVICE_TRACE(1, "------CPU frequency: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SDIO_DEVICE_TRACE(0, "------sdio device send an int to the host, send cnt:%d", wakeup_cnt);
#elif defined(ENABLE_SDIO_READ_REG_TEST)
    uint8_t crc8;

    wakeup_cnt++;
    if (wakeup_cnt > 0xFFFFFF) {
        wakeup_cnt = 1;
    }
    crc8 = crc8_itu((uint8_t *)&wakeup_cnt, 3);
    status = hal_sdio_device_send(sdio_id, (const uint8_t *)tx_buffer, TXRX_BUFF_SIZE, ((uint32_t)crc8 << 24) | wakeup_cnt);
    SDIO_DEVICE_TRACE(0, "------sdio device send an int to the host, send cnt:(0x%02X)%d", crc8, wakeup_cnt);
#elif defined(ENABLE_TX_SELF_INCREASE)
    static uint32_t tx_cnt = 2;

    status = hal_sdio_device_send(sdio_id, (const uint8_t *)tx_buffer, TXRX_BUFF_SIZE, tx_cnt);
    tx_cnt += 2;
    if (tx_cnt > TXRX_BUFF_SIZE) {
        tx_cnt = 2;
    }
#else //ENABLE_SDIO_WAKEUP_TEST
    status = hal_sdio_device_send(sdio_id, (const uint8_t *)tx_buffer, TXRX_BUFF_SIZE, TXRX_BUFF_SIZE);
#endif

#else //ENABLE_PEAK_BW_TX_TEST

    status = hal_sdio_device_send(sdio_id, (const uint8_t *)tx_buffer, TXRX_BUFF_SIZE, PEAK_DATA_SIZE_MIN + hal_sys_timer_get() % 101);//generate random numbers between 1600 and 1700
#endif

    if (status != HAL_SDIO_DEVICE_IDLE) {
        SDIO_DEVICE_TRACE(1, "******sdio device tx fail,device is not idle,status:%d", status);
    }
}

static struct HAL_SDIO_DEVICE_CB_T sdio_device_test_callback = {
    hal_sdio_device_enum_done,
    hal_sdio_device_tx_start,
    hal_sdio_device_rx_start,
    hal_sdio_device_send_done,
    hal_sdio_device_recv_done,
    hal_sdio_device_int_from_host,
};

#ifdef ENABLE_GPIO_WAKEUP_TEST
#define WAKEUP_PIN   HAL_GPIO_PIN_P1_2
static void wakeup_pin_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    POSSIBLY_UNUSED bool gpio_lvl;
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;

    wakeup_cnt++;
    gpio_lvl = hal_gpio_pin_get_val(pin);
    SDIO_DEVICE_TRACE(2, "%s, io=%d val=%d, wakeup cnt:%d", __func__, pin, gpio_lvl, wakeup_cnt);

    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_polarity = gpio_lvl ? HAL_GPIO_IRQ_POLARITY_LOW_FALLING : HAL_GPIO_IRQ_POLARITY_HIGH_RISING;;
    gpiocfg.irq_handler = wakeup_pin_irq_handler;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    hal_gpio_setup_irq(WAKEUP_PIN, &gpiocfg);

    //release the cpu, put it to sleep
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    TRACE(1, "CPU frequency: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
}

static void sdio_device_wakeup_pin(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP ext_pin[] = {
        {WAKEUP_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},//pull down
    };
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;

    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)ext_pin, sizeof(ext_pin) / sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
    hal_gpio_pin_set_dir(WAKEUP_PIN, HAL_GPIO_DIR_IN, 1);

    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;//wake-up high
    gpiocfg.irq_handler = wakeup_pin_irq_handler;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;    //Level wake-up mode
    hal_gpio_setup_irq(WAKEUP_PIN, &gpiocfg);
}
#endif

#define TEST_RESULT_ADDR                0x400000F4
#define HW_WRITE_REG(_Reg,_Value)       (*((volatile uint32_t*)(_Reg)) = (uint32_t)(_Value))
#define HW_READ_REG(_Reg)               (*((volatile uint32_t*)(_Reg)))
void sdio_device_test(void)
{
    uint32_t i;
    uint16_t *write_p = (uint16_t *)tx_buffer;

    SDIO_DEVICE_TRACE(0, "   ");
    SDIO_DEVICE_TRACE(0, "------sdio device test started------");
    //for rom simulation,2021-08-09
    SDIO_DEVICE_TRACE(1, "------test result_start[0x%X]:0x%X", TEST_RESULT_ADDR, HW_READ_REG(TEST_RESULT_ADDR));
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_208M);
    SDIO_DEVICE_TRACE(0, "   ");
    SDIO_DEVICE_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);

    for (i = 0; i < TXRX_BUFF_SIZE / 2; i++) {
        write_p[i] = i;
    }

    /* sdio device iomux */
    hal_iomux_set_sdio();
#ifndef ENABLE_APP_MODE
    hal_sdio_device_open(sdio_id, &sdio_device_test_callback);
#else
    hal_sdio_device_reopen(sdio_id, &sdio_device_test_callback);
#endif

    for (i = 0; i < TXRX_BUFF_CNT; i++) {
        memset((uint8_t *)&rx_buffer[i][0], FORMATTED_DATA, TXRX_BUFF_SIZE);
    }
    hal_sdio_device_recv(sdio_id, &rx_buffer[rxbuf_index][0], TXRX_BUFF_SIZE, TXRX_BUFF_CNT);

    //for rom simulation,2021-08-09
    HW_WRITE_REG(TEST_RESULT_ADDR, 0x9A55);
    SDIO_DEVICE_TRACE(1, "------test result_end[0x%X]:0x%X", TEST_RESULT_ADDR, HW_READ_REG(TEST_RESULT_ADDR));

    //hal_iomux_set_sdio_voltage(HAL_IOMUX_PIN_VOLTAGE_VIO);
    SDIO_DEVICE_TRACE(1, "******volt switch reg:%s", hal_iomux_get_sdio_voltage() ? "VMEM[1]" : "VIO[0]");
    SDIO_DEVICE_TRACE(0, "------sdio device waiting interrupt");
    SDIO_DEVICE_TRACE(0, "  ");

#ifdef ENABLE_GPIO_WAKEUP_TEST
    sdio_device_wakeup_pin();
#endif

    while (1) {
#ifdef ENABLE_SDIO_WAKEUP_TEST
        if (wakeup_cnt) {
            SDIO_DEVICE_TRACE(0, "------sdio device goes to sleep");
            break;//Enter sleep after the sdio device is successfully enumerated
        }
#endif

#ifdef ENABLE_GPIO_WAKEUP_TEST
        break;
#endif
    }
}

#endif

