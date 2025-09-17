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
#include "hal_tests.h"

#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "hal_gpadc.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"

extern "C" {
void hal_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);
void hal_timer_start(uint32_t load);
void hal_timer_stop(void);
}

#define GPIO_WAKEUP_TEST_PIN                (HAL_IOMUX_PIN_P2_0)
#define UART_BAUD_DEFAULT                   (1152000)
#define UART_TX_STRING_1                    "This from UART1...\r\n"
#define RX_BUF_MAX                          (256)
#define GPADC_TEST_MODE                     HAL_GPADC_ATP_ONESHOT
#define ONESHOT_INTERVAL_TIME               (1000)
#define GPADC_TRACE(str, ...)               TRACE(TR_MOD(HAL), str, ##__VA_ARGS__)
#define GPADC_TRACE_VAL(str, ...)           TRACE_IMM(TR_MOD(HAL), str, ##__VA_ARGS__)
#define ONESLOT_MS_DEFAULT                  (2000)
#define TEST_ONESLOT_DELAY_DEFAULT          (ONESLOT_MS_DEFAULT + 500)
#define PERIODIC_MS_DEFAULT                 (1000)
#define TEST_PERIODIC_DELAY_DEFAULT         (PERIODIC_MS_DEFAULT * 3)

static void gpio_test_irq_handler(enum HAL_GPIO_PIN_T pin);
static void test_uart_dma_tx_irq_handler(uint32_t xfer_size, int dma_error);
static void test_uart_dma_rx_irq_handler(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status);
static void gpadc_delay(uint32_t delay_ms);
static void gpadc_ch0_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt);
static void slow_timer_irq_handler(uint32_t elapsed);
static void slow_timer_periodic_irq_handler(uint32_t elapsed);

static bool g_tx_dma_done = false;
static uint8_t g_test_rx_buf[RX_BUF_MAX] = {0};
static osThreadId_t test_timer_thread_id = NULL;
static uint32_t slow_timer_old_time = 0;
static uint32_t slow_timer_new_time = 0;

static void gpio_test_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    ASSERT(pin == (enum HAL_GPIO_PIN_T)GPIO_WAKEUP_TEST_PIN, "%s: Bad test gpio: %u (expecting %u)",
        __func__, pin, GPIO_WAKEUP_TEST_PIN);
    TRACE(0, "GPIO %u IRQ happes!", pin);
}

void gpio_wakeup_test(char* cmdParam, uint32_t cmdParam_len)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP test_pin[] =
    {
        {
            .pin = GPIO_WAKEUP_TEST_PIN,
            .function = HAL_IOMUX_FUNC_AS_GPIO,
            .volt = HAL_IOMUX_PIN_VOLTAGE_VIO,
            .pull_sel = HAL_IOMUX_PIN_PULLDOWN_ENABLE,
        },
    };
    hal_iomux_init(&test_pin[0], sizeof(test_pin) / sizeof(HAL_IOMUX_PIN_FUNCTION_MAP));
    const struct HAL_GPIO_IRQ_CFG_T test_gpio_cfg =
    {
        .irq_enable = true,
        .irq_debounce = true,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = gpio_test_irq_handler,
    };
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)GPIO_WAKEUP_TEST_PIN, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)GPIO_WAKEUP_TEST_PIN, &test_gpio_cfg);
}

static void test_uart_dma_tx_irq_handler(uint32_t xfer_size, int dma_error)
{
    if (dma_error)
    {
        TRACE(0, "DMA error");
    }
    g_tx_dma_done = true;
    TRACE(0,"%s, tx done",__func__);
}

static void test_uart_dma_rx_irq_handler(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    if (dma_error)
    {
        TRACE(0, "DMA error");
    } else
    {
        if (g_tx_dma_done)
        {
            TRACE(0,"%s, __line__%d",__func__, __LINE__);
            g_tx_dma_done = 0;
            hal_uart_dma_send(HAL_UART_ID_1, g_test_rx_buf, xfer_size, NULL, NULL);
        }
        {
        union HAL_UART_IRQ_T mask;
        mask.reg = 0;
        mask.BE = 1;
        mask.FE = 1;
        mask.OE = 1;
        mask.PE = 0;
        mask.RT = 1;

        hal_uart_dma_recv_mask(HAL_UART_ID_1, g_test_rx_buf, RX_BUF_MAX, NULL, NULL, &mask);
        }
    }
}

void uart1_demo_test(char* cmdParam, uint32_t cmdParam_len)
{
    hal_iomux_set_uart1();
    POSSIBLY_UNUSED int ret = 0;
    hal_uart_close(HAL_UART_ID_1);
    struct HAL_UART_CFG_T uart_dma_cfg =
    {
        .parity             = HAL_UART_PARITY_NONE,
        .stop               = HAL_UART_STOP_BITS_1,
        .data               = HAL_UART_DATA_BITS_8,
        .flow               = HAL_UART_FLOW_CONTROL_NONE,
        .rx_level           = HAL_UART_FIFO_LEVEL_1_4,
        .tx_level           = HAL_UART_FIFO_LEVEL_1_2,
        .baud               = UART_BAUD_DEFAULT,
        .dma_rx             = true,
        .dma_tx             = true,
        .dma_rx_stop_on_err = false,
    };
    ret = hal_uart_open(HAL_UART_ID_1, &uart_dma_cfg);
    hal_uart_irq_set_dma_handler(HAL_UART_ID_1, test_uart_dma_rx_irq_handler, test_uart_dma_tx_irq_handler);
    {
    union HAL_UART_IRQ_T mask;
    mask.reg = 0;
    mask.BE = 1;
    mask.FE = 1;
    mask.OE = 1;
    mask.PE = 0;
    mask.RT = 1;
    ret = hal_uart_dma_recv_mask(HAL_UART_ID_1, g_test_rx_buf, RX_BUF_MAX, NULL, NULL, &mask);
    }
    hal_uart_dma_send(HAL_UART_ID_1, (uint8_t *)"send the string on 10 times.\r\n",
                            sizeof("send the string on 10 times.\r\n"), NULL, NULL);
    for (uint8_t i = 0; i < 10; ++i)
    {
        hal_uart_dma_send(HAL_UART_ID_1, (uint8_t *)UART_TX_STRING_1, sizeof(UART_TX_STRING_1), NULL, NULL);
        osDelay(100U);
    }
    do {
        osDelay(1000U);
    } while(1);
    union HAL_UART_IRQ_T mask;
    mask.reg = 0;
    hal_uart_stop_dma_recv(HAL_UART_ID_1);
    hal_uart_irq_set_mask(HAL_UART_ID_1, mask);
    ret = hal_uart_close(HAL_UART_ID_1);
    TRACE(0, "uart1_demo_test END");
}

static void gpadc_delay(uint32_t delay_ms)
{
    hal_sys_timer_delay(MS_TO_TICKS(delay_ms));
}

static void gpadc_ch0_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d ", __func__, irq_val, volt);
}

void gpadc_test(char* cmdParam, uint32_t cmdParam_len)
{
    if (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT)
    {
        GPADC_TRACE("gpadc uses oneshot mode, oneshot interval time is %d ms", ONESHOT_INTERVAL_TIME);
    } else
    {
        GPADC_TRACE("gpadc uses continuous mode");
    }
    int ret = 0;
    do {
        ret = hal_gpadc_open(HAL_GPADC_CHAN_0, GPADC_TEST_MODE, gpadc_ch0_irq_handler);
        ASSERT(0 == ret, "gpadc ch0 init fail, ret=%d", ret);
        if (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT)
        {
            gpadc_delay(ONESHOT_INTERVAL_TIME);
            TRACE_OUTPUT_LINEFEED();
            TRACE_OUTPUT_LINEFEED();
        }
    } while (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT);
}

static void slow_timer_irq_handler(uint32_t elapsed)
{
    TRACE(2,"%s , 0x%x", __FUNCTION__, elapsed);
    osSignalSet(test_timer_thread_id, 1);
}

static void slow_timer_periodic_irq_handler(uint32_t elapsed)
{
    slow_timer_new_time = hal_fast_sys_timer_get();
    if (slow_timer_old_time)
    {
        TRACE(2, "slow_timer periodic run time: %dms", FAST_TICKS_TO_MS(slow_timer_new_time - slow_timer_old_time));
    }
    TRACE(2,"%s , 0x%x", __FUNCTION__, elapsed);
    slow_timer_old_time = slow_timer_new_time;
}

void hw_timer_test(char* cmdParam, uint32_t cmdParam_len)
{
    osEvent evt;
    uint32_t startTime;
    uint32_t endTime;
    test_timer_thread_id = osThreadGetId();

    /* oneshot mode */
    TRACE(3,"%s timer oneshot [%dms] start.", __FUNCTION__, ONESLOT_MS_DEFAULT);
    hal_timer_setup(HAL_TIMER_TYPE_ONESHOT, slow_timer_irq_handler);
    startTime = hal_sys_timer_get();
    hal_timer_start(MS_TO_TICKS(ONESLOT_MS_DEFAULT));

    if (NULL != test_timer_thread_id)
    {
        evt = osSignalWait(osOK, 5000);
        endTime = hal_sys_timer_get();
        if (evt.status == osEventSignal)
        {
            TRACE(2, "Check hardware timer freerun time: %dms", endTime - startTime);
        }
        else
        {
            TRACE(2, "osEventTimeout");
        }
    }

    /* periodic mode */
    TRACE(3,"%s timer periodic[%dms] start.", __FUNCTION__, PERIODIC_MS_DEFAULT);
    hal_timer_setup(HAL_TIMER_TYPE_PERIODIC, slow_timer_periodic_irq_handler);
    hal_timer_start(MS_TO_TICKS(PERIODIC_MS_DEFAULT));
}
