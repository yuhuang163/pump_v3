/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "hwtest.h"
#include "hal_cmu.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer_raw.h"
#include "hal_uart.h"
#include "hwtimer_list.h"
#include "pmu.h"

// SLEEP_TEST

#ifndef NO_WAKEUP
#ifndef NO_TIMER
#define TIMER_WAKEUP
#endif
//#define RTC_WAKEUP
//#define GPIO_WAKEUP
//#define UART_WAKEUP
#endif

#define TIMER_IRQ_SERVICE_MASK          (1 << 0)
#define RTC_IRQ_SERVICE_MASK            (1 << 1)
#define GPIO_IRQ_SERVICE_MASK           (1 << 2)
#define UART_IRQ_SERVICE_MASK           (1 << 3)

static uint32_t irq_service_mask;
static uint32_t irq_check_mask;
static HWTIMER_CALLBACK_T timer_cb[3];
static void (*loop_cb[3])(void);

void sleep_test_register_timer_callback(HWTIMER_CALLBACK_T cb)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(timer_cb); i++) {
        if (!timer_cb[i]) {
            timer_cb[i] = cb;
            return;
        }
    }

    ASSERT(false, "%s: failed", __func__);
}

POSSIBLY_UNUSED
static void sleep_test_exec_timer_callback(void *param)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(timer_cb); i++) {
        if (timer_cb[i]) {
            timer_cb[i](param);
        }
    }
}

void sleep_test_register_loop_callback(void (*cb)(void))
{
    for (uint32_t i = 0; i < ARRAY_SIZE(loop_cb); i++) {
        if (!loop_cb[i]) {
            loop_cb[i] = cb;
            return;
        }
    }

    ASSERT(false, "%s: failed", __func__);
}

POSSIBLY_UNUSED
static void sleep_test_exec_loop_callback(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(loop_cb); i++) {
        if (loop_cb[i]) {
            loop_cb[i]();
        }
    }
}

#ifdef TIMER_WAKEUP
static HWTIMER_ID wakeup_timer;
static uint32_t wakeup_cnt;
static const uint32_t timer_ms = 17;

static void timer_handler(void *param)
{
    if (wakeup_cnt++ < 1) {
        TRACE(0, "%s", __FUNCTION__);
    }
    irq_service_mask |= TIMER_IRQ_SERVICE_MASK;

    sleep_test_exec_timer_callback(param);

    hwtimer_start(wakeup_timer, MS_TO_TICKS(timer_ms));
}
#endif

#ifdef RTC_WAKEUP
static void rtc_handler(uint32_t seconds)
{
    TRACE(0, "%s: [%d] seconds=%d", __FUNCTION__, TICKS_TO_MS(hal_sys_timer_get()));
    irq_service_mask |= RTC_IRQ_SERVICE_MASK;
}
#endif

#ifdef GPIO_WAKEUP
static void gpio_handler(enum HAL_GPIO_PIN_T pin)
{
    TRACE(0, "%s: %d", __FUNCTION__, pin);
    irq_service_mask |= GPIO_IRQ_SERVICE_MASK;
}
#endif

#if defined(UART_WAKEUP) && defined(UART1_BASE)
static void uart_handler(enum HAL_UART_ID_T id, union HAL_UART_IRQ_T status)
{
    union HAL_UART_IRQ_T mask;
    mask.reg = 0;
    hal_uart_irq_set_mask(HAL_UART_ID_1, mask);
    irq_service_mask |= UART_IRQ_SERVICE_MASK;
}

static const struct HAL_UART_CFG_T uart_cfg = {
    .parity = HAL_UART_PARITY_NONE,
    .stop = HAL_UART_STOP_BITS_1,
    .data = HAL_UART_DATA_BITS_8,
    .flow = HAL_UART_FLOW_CONTROL_NONE,//HAL_UART_FLOW_CONTROL_RTSCTS,
    .tx_level = HAL_UART_FIFO_LEVEL_1_2,
    .rx_level = HAL_UART_FIFO_LEVEL_1_4,
    .baud = 921600,
    .dma_rx = false,
    .dma_tx = false,
    .dma_rx_stop_on_err = false,
};
#endif

void sleep_test(void)
{
    enum HAL_SLEEP_STATUS_T result;

#if defined(ROM_BUILD) && !defined(NO_PMU)
    // Allow sleep
#ifdef PMU_FULL_INIT
    pmu_open();
#else
    pmu_sleep_en(1);
#endif
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);

#ifdef TIMER_WAKEUP
    // Timer
    wakeup_timer = hwtimer_alloc(timer_handler, NULL);
    ASSERT(wakeup_timer, "Failed to alloc wakeup_timer");
    int ret = hwtimer_start(wakeup_timer, MS_TO_TICKS(timer_ms));
    ASSERT(ret == E_HWTIMER_OK, "Failed to start hwtimer: %d" ,ret);
    irq_check_mask |= TIMER_IRQ_SERVICE_MASK;
#endif

#ifdef RTC_WAKEUP
    // RTC
    pmu_rtc_enable();
    pmu_rtc_set(0);
    pmu_rtc_set_irq_handler(rtc_handler);
    pmu_rtc_set_alarm(2);
    irq_check_mask |= RTC_IRQ_SERVICE_MASK;
#endif

#ifdef GPIO_WAKEUP
    // GPIO_P0_0
    enum HAL_GPIO_PIN_T gpio_pin = HAL_GPIO_PIN_P0_0;
    struct HAL_GPIO_IRQ_CFG_T gpio_cfg;
    hal_iomux_set_io_pull_select((enum HAL_IOMUX_PIN_T)gpio_pin, HAL_IOMUX_PIN_PULLUP_ENABLE);
    hal_gpio_pin_set_dir(gpio_pin, HAL_GPIO_DIR_IN, 0);
    gpio_cfg.irq_enable = true;
    gpio_cfg.irq_debounce = true;
    // Choose edge irq to avoid the irq timing issue in the simulation environment.
    // (Level irq will disappear if gpio value goes back to high too fast, or it will be
    // raised again if gpio value keeps low too long)
    gpio_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpio_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    gpio_cfg.irq_handler = gpio_handler;
    hal_gpio_setup_irq(gpio_pin, &gpio_cfg);
    irq_check_mask |= GPIO_IRQ_SERVICE_MASK;
#endif

#if defined(UART_WAKEUP) && defined(UART1_BASE)
    // UART1
    union HAL_UART_IRQ_T mask;
    hal_uart_open(HAL_UART_ID_1, &uart_cfg);
    mask.reg = 0;
    mask.BE = 1;
    hal_uart_irq_set_mask(HAL_UART_ID_1, mask);
    hal_uart_irq_set_handler(HAL_UART_ID_1, uart_handler);
    irq_check_mask |= UART_IRQ_SERVICE_MASK;
#endif

    while (1) {
        sleep_test_exec_loop_callback();
        result = hal_sleep_enter_sleep();
        if (result == HAL_SLEEP_STATUS_DEEP) {
            if (irq_check_mask && irq_check_mask == irq_service_mask) {
                hal_cmu_simu_pass();
            }
        }
    }
}

