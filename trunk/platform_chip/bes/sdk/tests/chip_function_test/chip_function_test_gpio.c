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
#ifdef CHIP_FUNCTION_TEST_GPIO
#include "cmsis.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "hal_trace.h"
 #include "string.h"
#ifdef RTOS
#include "cmsis_os.h"
#ifdef KERNEL_RTX
#include "rt_Time.h"
#endif
#endif
#include "hal_gpio.h"

#ifdef TEST_GPIO_IN_05
#define GPIO_IN                    HAL_GPIO_PIN_P0_5
#else
#define GPIO_IN                    HAL_GPIO_PIN_P0_0
#endif

#ifdef TEST_GPIO_OUT_07
#define GPIO_OUT                   HAL_GPIO_PIN_P0_7
#else
#define GPIO_OUT                    HAL_GPIO_PIN_P0_1
#endif

#ifndef SQUARE_WAVE_TIME_CYCLE_MS
#define SQUARE_WAVE_TIME_CYCLE_MS  100
#endif

#ifdef SQUARE_WAVE_OUTPUT
static uint32_t _output_time = 0;
static uint32_t _total_time = 0;
static uint32_t _total_times = 0;
static void _meauring_gpio_irqhandler(enum HAL_GPIO_PIN_T pin)
{
    static uint32_t ind = 0;
    uint32_t cur_time = hal_fast_sys_timer_get();
    uint32_t wait_time;

    wait_time = FAST_TICKS_TO_US(cur_time - _output_time);

    _total_time += wait_time;
    _total_times ++;

    //if(ind % 100 == 0)
    {
        TRACE(0, "MEAURING: %s WAKEUP TIME = %d(us) , ind = %d", __func__, _total_time/_total_times, ind++);
    }
}

static void _output_square_wave(enum HAL_GPIO_PIN_T out_pin,
                         uint32_t delay_ms,
                         bool is_once)
{
    uint32_t out_cnt = 0;
    TRACE(0, "%s: out_pin = %d, delay_ms = %d, is_once = %d", __func__, out_pin, delay_ms, is_once);
    while(1)
    {
        hal_gpio_pin_set(out_pin);
        _output_time = hal_fast_sys_timer_get();

        osDelay(delay_ms);
        hal_gpio_pin_clr(out_pin);
        osDelay(delay_ms);
        if(out_cnt % 100 == 0){
            TRACE(0, "set output signal, _out_cnt = %d", out_cnt);
            //hal_sys_timer_delay_us(10000);
        }
        out_cnt++;

        if(is_once)
        {
            break;
        }
    }
}

void function_test_square_wave_output(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_cfg;
    enum HAL_GPIO_PIN_T pin_in;
    enum HAL_GPIO_PIN_T pin_out;
    uint32_t delay_ms = SQUARE_WAVE_TIME_CYCLE_MS/2;
    bool is_once;
    static struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {GPIO_IN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {GPIO_OUT, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_NOPULL},
    };

    TRACE(1, "GPIO SQUARE WAVE OUTPUT: BEGIN.");
#ifdef SQUARE_WAVE_OUTPUT_ONCE
    is_once = true;
#else
    is_once = false;
#endif
    pin_in = pinmux[0].pin;
    pin_out = pinmux[1].pin;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(pin_out, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(pin_in, HAL_GPIO_DIR_IN, 0);


    gpio_cfg.irq_enable = true;
    gpio_cfg.irq_debounce = true;
    gpio_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpio_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpio_cfg.irq_handler = _meauring_gpio_irqhandler;
    hal_gpio_setup_irq(pin_in, &gpio_cfg);

    _output_square_wave(pin_out, delay_ms, is_once);

}
#endif

#ifdef GPIO_IRQ_WAKEUP_TEST

extern void hal_timer_start(uint32_t load);
extern void hal_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);
extern void hal_user_timer0_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);
extern void hal_user_timer0_start(uint32_t load);
extern void hal_user_timer0_stop(void);

extern void hal_user_timer1_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);
extern void hal_user_timer1_start(uint32_t load);
extern void hal_user_timer1_stop(void);


#define WAKEUP_TEST_GPIO_OUT GPIO_OUT

#if defined(IRQ_WAKEUP_TIMER0_IRQ0) || defined(IRQ_WAKEUP_TIMER0_IRQ1)
#define TIMER_START_GPIO_OUT  GPIO_IN
#define WAKEUP_TEST_GPIO_IN  GPIO_IN
#elif defined(IRQ_WAKEUP_GPIO_82_IRQ)
#define WAKEUP_TEST_GPIO_IN  HAL_GPIO_PIN_P8_2
#elif defined(IRQ_WAKEUP_GPIO_04_IRQ)
#define WAKEUP_TEST_GPIO_IN  HAL_GPIO_PIN_P0_4
#elif defined(IRQ_WAKEUP_GPIO_83_IRQ)
#define WAKEUP_TEST_GPIO_IN  HAL_GPIO_PIN_P8_3
#elif defined(IRQ_WAKEUP_GPIO_05_IRQ)
#define WAKEUP_TEST_GPIO_IN  HAL_GPIO_PIN_P0_5
#elif defined(IRQ_WAKEUP_GPIO_85_IRQ)
#define WAKEUP_TEST_GPIO_IN  HAL_GPIO_PIN_P8_5
#elif defined(IRQ_WAKEUP_GPIO_84_IRQ)
#define WAKEUP_TEST_GPIO_IN  HAL_GPIO_PIN_P8_4
#elif defined(IRQ_WAKEUP_GPIO_87_IRQ)
#define WAKEUP_TEST_GPIO_IN  HAL_GPIO_PIN_P8_7
#elif defined(IRQ_WAKEUP_GPIO_86_IRQ)
#define WAKEUP_TEST_GPIO_IN  HAL_GPIO_PIN_P8_6
#else
#define WAKEUP_TEST_GPIO_IN  GPIO_IN
#endif

static void _irqhandler(enum HAL_GPIO_PIN_T pin)
{
    static uint32_t ind = 0;
    static bool is_set = true;
    if(is_set)
    {
        hal_gpio_pin_set(WAKEUP_TEST_GPIO_OUT);
    }
    else
    {
        hal_gpio_pin_clr(WAKEUP_TEST_GPIO_OUT);
    }
    is_set = !is_set;
    if(ind++ % 20 == 0)
    {
        TRACE(0, "GPIO IRQ WAKEUP: %s pin = %d, ind = %d", __func__, pin, ind);
    }
}

void gpio_irq_wakeup(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_cfg;
    enum HAL_GPIO_PIN_T pin_in;
    enum HAL_GPIO_PIN_T pin_out;
    static struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {WAKEUP_TEST_GPIO_IN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {WAKEUP_TEST_GPIO_OUT, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_NOPULL},
    };

    TRACE(1, "GPIO IRQ WAKEUP: BEGIN.");
    pin_in = pinmux[0].pin;
    pin_out = pinmux[1].pin;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(pin_out, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(pin_in, HAL_GPIO_DIR_IN, 0);


    gpio_cfg.irq_enable = true;
    gpio_cfg.irq_debounce = true;
    gpio_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpio_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpio_cfg.irq_handler = _irqhandler;
    hal_gpio_setup_irq(pin_in, &gpio_cfg);

}

#if defined(IRQ_WAKEUP_TIMER0_IRQ0) || defined(IRQ_WAKEUP_TIMER0_IRQ1)
static HWTIMER_ID _hw_timer = NULL;

static void _timer_irq_handler(void *param)
{
    static uint32_t ind = 0;
    static bool is_set = true;
    if(is_set)
    {
        hal_gpio_pin_set(WAKEUP_TEST_GPIO_OUT);
        hal_gpio_pin_clr(TIMER_START_GPIO_OUT);
    }
    else
    {
        hal_gpio_pin_clr(WAKEUP_TEST_GPIO_OUT);
        hal_gpio_pin_set(TIMER_START_GPIO_OUT);
    }
    is_set = !is_set;

#if defined(IRQ_WAKEUP_TIMER0_IRQ0)
    hwtimer_start(_hw_timer, MS_TO_TICKS(SQUARE_WAVE_TIME_CYCLE_MS));
#endif
#if defined(IRQ_WAKEUP_TIMER0_IRQ1)
    hwtimer_start(_hw_timer, MS_TO_TICKS(SQUARE_WAVE_TIME_CYCLE_MS));
#endif

    if(ind++ % 20 == 0)
    {
        TRACE(0, "TIMER IRQ WAKEUP: %s ind = %d", __func__, ind);
    }
}

void timer_irq_test(void)
{
    TRACE(1, "TIMER IRQ WAKEUP TEST: BEGIN.");

    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {(enum HAL_IOMUX_PIN_T)TIMER_START_GPIO_OUT, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM,
HAL_IOMUX_PIN_NOPULL},
        {(enum HAL_IOMUX_PIN_T)WAKEUP_TEST_GPIO_OUT, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM,
HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(WAKEUP_TEST_GPIO_OUT, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(TIMER_START_GPIO_OUT, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_clr(WAKEUP_TEST_GPIO_OUT);
    hal_gpio_pin_clr(TIMER_START_GPIO_OUT);
    hal_sys_timer_delay(MS_TO_TICKS(SQUARE_WAVE_TIME_CYCLE_MS)*2);
    hal_gpio_pin_set(TIMER_START_GPIO_OUT);
#if defined(IRQ_WAKEUP_TIMER0_IRQ0)
    _hw_timer = hwtimer_alloc(_timer_irq_handler, 0);
    hwtimer_start(_hw_timer, MS_TO_TICKS(SQUARE_WAVE_TIME_CYCLE_MS));

#endif
#if defined(IRQ_WAKEUP_TIMER0_IRQ1)
    _hw_timer = hwtimer_alloc(_timer_irq_handler, 0);
    hwtimer_start(_hw_timer,MS_TO_TICKS(SQUARE_WAVE_TIME_CYCLE_MS));
#endif
    hal_gpio_pin_clr(TIMER_START_GPIO_OUT);

}
#endif

#define PMU_GPIO_FAST_IRQ_REG  0x39
#define PMU_P82_04_FAST_IRQ_SELECT_SHIFT 4
#define PMU_P82_FAST_IRQ_SLELECT 0x1
#define PMU_P04_FAST_IRQ_SLELECT 0x0
#define PMU_P83_05_FAST_IRQ_SELECT_SHIFT 6
#define PMU_P83_FAST_IRQ_SLELECT 0x1
#define PMU_P05_FAST_IRQ_SLELECT 0x0
#define PMU_P85_84_FAST_IRQ_SELECT_SHIFT 8
#define PMU_P85_FAST_IRQ_SLELECT 0x1
#define PMU_P84_FAST_IRQ_SLELECT 0x0
#define PMU_P87_86_FAST_IRQ_SELECT_SHIFT 10
#define PMU_P87_FAST_IRQ_SLELECT 0x1
#define PMU_P86_FAST_IRQ_SLELECT 0x0

#define PMU_P82_04_FAST_IRQ_ENABLE_SHIFT 0
#define PMU_P83_05_FAST_IRQ_ENABLE_SHIFT 1
#define PMU_P85_84_FAST_IRQ_ENABLE_SHIFT 2
#define PMU_P87_86_FAST_IRQ_ENABLE_SHIFT 3
#define PMU_TIEMR_FAST_IRQ_ENABLE_SHIFT  14
#define PMU_FAST_IRQ_ENABLE  0x1
#define PMU_FAST_IRQ_DISABLE 0x0

#define PMU_P82_04_FAST_IRQ_POLARITY_SHIFT 5
#define PMU_P83_05_FAST_IRQ_POLARITY_SHIFT 7
#define PMU_P85_84_FAST_IRQ_POLARITY_SHIFT 9
#define PMU_P87_86_FAST_IRQ_POLARITY_SHIFT 11
#define PMU_FAST_IRQ_POLARITY_HIGH_RISING 0x0
#define PMU_FAST_IRQ_POLARITY_LOW_FALLING 0x1

#define FAST_IRQ_ENALBE_SHIFT 0
#define AON_TIMER0_IRQ0_ENALBE_SHIFT 1
#define AON_TIMER0_IRQ1_ENALBE_SHIFT 2
#define AON_TIMER0_ENABLE  0x1
#define AON_TIMER0_DISABLE 0x0

#include "pmu.h"
#define TIMER0_GPIO_FAST_IRQ_REG 0x400800F0
void function_test_gpio_irq_wakeup(void)
{
    uint32_t *dig_reg = (uint32_t*)TIMER0_GPIO_FAST_IRQ_REG;
    uint32_t dig_val;
    unsigned short pmu_val = 0;

    dig_val = *dig_reg;
    TRACE(1,"%s: ORAGINAL DIG_REG(0x%x) = 0x%x", __func__, (uint32_t)dig_reg, *dig_reg);
    pmu_read(PMU_GPIO_FAST_IRQ_REG, &pmu_val);
    TRACE(1,"%s: ORAGINAL PMU_REG(0x%x) = 0x%x", __func__, PMU_GPIO_FAST_IRQ_REG, pmu_val);
#if defined(IRQ_WAKEUP_GPIO_82_IRQ)
    pmu_val |= (0x1 << PMU_P82_04_FAST_IRQ_SELECT_SHIFT);
    pmu_val |= (0x1 << PMU_P82_04_FAST_IRQ_ENABLE_SHIFT);
    pmu_val &= ~(0x1 << PMU_P82_04_FAST_IRQ_POLARITY_SHIFT);
#elif defined(IRQ_WAKEUP_GPIO_04_IRQ)
    pmu_val &= (0x1 << PMU_P82_04_FAST_IRQ_SELECT_SHIFT);
    pmu_val |= (0x1 << PMU_P82_04_FAST_IRQ_ENABLE_SHIFT);
    pmu_val &= ~(0x1 << PMU_P82_04_FAST_IRQ_POLARITY_SHIFT);
#elif defined(IRQ_WAKEUP_GPIO_83_IRQ)
    pmu_val |= (0x1 << PMU_P83_05_FAST_IRQ_SELECT_SHIFT);
    pmu_val |= (0x1 << PMU_P83_05_FAST_IRQ_ENABLE_SHIFT);
    pmu_val &= ~(0x1 << PMU_P83_05_FAST_IRQ_POLARITY_SHIFT);
#elif defined(IRQ_WAKEUP_GPIO_05_IRQ)
    pmu_val &= (0x1 << PMU_P83_05_FAST_IRQ_SELECT_SHIFT);
    pmu_val |= (0x1 << PMU_P83_05_FAST_IRQ_ENABLE_SHIFT);
    pmu_val &= ~(0x1 << PMU_P83_05_FAST_IRQ_POLARITY_SHIFT);
#elif defined(IRQ_WAKEUP_GPIO_85_IRQ)
    pmu_val |= (0x1 << PMU_P85_84_FAST_IRQ_SELECT_SHIFT);
    pmu_val |= (0x1 << PMU_P85_84_FAST_IRQ_ENABLE_SHIFT);
    pmu_val &= ~(0x1 << PMU_P85_84_FAST_IRQ_POLARITY_SHIFT);
#elif defined(IRQ_WAKEUP_GPIO_84_IRQ)
    pmu_val &= (0x1 << PMU_P85_84_FAST_IRQ_SELECT_SHIFT);
    pmu_val |= (0x1 << PMU_P85_84_FAST_IRQ_ENABLE_SHIFT);
    pmu_val &= ~(0x1 << PMU_P85_84_FAST_IRQ_POLARITY_SHIFT);
#elif defined(IRQ_WAKEUP_GPIO_87_IRQ)
    pmu_val |= (0x1 << PMU_P87_86_FAST_IRQ_SELECT_SHIFT);
    pmu_val |= (0x1 << PMU_P87_86_FAST_IRQ_ENABLE_SHIFT);
    pmu_val &= ~(0x1 << PMU_P87_86_FAST_IRQ_POLARITY_SHIFT);
#elif defined(IRQ_WAKEUP_GPIO_86_IRQ)
    pmu_val &= (0x1 << PMU_P87_86_FAST_IRQ_SELECT_SHIFT);
    pmu_val |= (0x1 << PMU_P87_86_FAST_IRQ_ENABLE_SHIFT);
    pmu_val &= ~(0x1 << PMU_P87_86_FAST_IRQ_POLARITY_SHIFT);

#else // TIMMER IRQ or GPIO 05 IRQ

#if defined(IRQ_WAKEUP_TIMER0_IRQ0) || defined(IRQ_WAKEUP_TIMER0_IRQ1) // TIMER IRQ
    pmu_val |= (0x1 << PMU_TIEMR_FAST_IRQ_ENABLE_SHIFT);
#if defined(IRQ_WAKEUP_TIMER0_IRQ0)
    dig_val |= (0x1 << AON_TIMER0_IRQ0_ENALBE_SHIFT);
#elif defined(IRQ_WAKEUP_TIMER0_IRQ0)
    dig_val |= (0x1 << AON_TIMER0_IRQ1_ENALBE_SHIFT);
#endif
#else // GPIO 05 IRQ
    gpio_irq_wakeup();
    return;
#endif
#endif
    // GPIO GROUPS 1-4 IRQ and TIMER IRQ:
    dig_val |= (0x1 << FAST_IRQ_ENALBE_SHIFT);
    *dig_reg = dig_val;
    TRACE(1,"%s: NEW DIG_REG(0x%x) = 0x%x", __func__, (uint32_t)dig_reg, *dig_reg);

    pmu_write(PMU_GPIO_FAST_IRQ_REG, pmu_val);
    pmu_read(PMU_GPIO_FAST_IRQ_REG, &pmu_val);
    TRACE(1,"%s: NEW PMU_REG(0x%x) = 0x%x", __func__, PMU_GPIO_FAST_IRQ_REG, pmu_val);
    TRACE_FLUSH();
#if defined(IRQ_WAKEUP_TIMER0_IRQ0) || defined(IRQ_WAKEUP_TIMER0_IRQ1)
    timer_irq_test();
#else
    gpio_irq_wakeup();
#endif
}
#endif

#ifdef GPIO_IRQ_TEST
static void _gpio_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    hal_gpio_pin_set(GPIO_OUT);
    TRACE(0, "%s: %d", __FUNCTION__, pin);
}

void gpio_irq_test(void)
{
    TRACE(1, "GPIO IRQ TEST");
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {(enum HAL_IOMUX_PIN_T)GPIO_IN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {(enum HAL_IOMUX_PIN_T)GPIO_OUT, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(GPIO_OUT, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(GPIO_IN, HAL_GPIO_DIR_IN, 0);

    struct HAL_GPIO_IRQ_CFG_T gpio_cfg;
    gpio_cfg.irq_enable = true;
    gpio_cfg.irq_debounce = true;
    gpio_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpio_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    gpio_cfg.irq_handler = _gpio_irq_handler;
    hal_gpio_setup_irq(GPIO_IN, &gpio_cfg);
    //TRACE(1, "GPIO TEST: DONE.");
}
#endif

void function_test_gpio(void)
{
#ifdef SQUARE_WAVE_OUTPUT
    function_test_square_wave_output();
#endif
#ifdef GPIO_IRQ_WAKEUP_TEST
    function_test_gpio_irq_wakeup();
#endif
#ifdef GPIO_IRQ_TEST
    gpio_irq_test();
    while(1);
#endif
}
#endif

