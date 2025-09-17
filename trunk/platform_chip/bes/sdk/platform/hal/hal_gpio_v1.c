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
#if (CHIP_GPIO_VER <= 1)

#include "plat_addr_map.h"

#ifdef GPIO_BASE

#include "cmsis_nvic.h"
#include "hal_gpio.h"
#ifdef PSC_GPIO_IRQ_CTRL
#include "hal_psc.h"
#endif
#include "hal_timer.h"
#include "hal_trace.h"
#ifdef PMU_HAS_LED_PIN
#include "pmu.h"
#endif
#include "reg_gpio_v1.h"

#define HAL_GPIO_PIN_NUM_EACH_PORT          (32)

#define HAL_GPIO_BANK_NUM                   ARRAY_SIZE(gpio_bank)

#define HAL_GPIO_PIN_NUM_EACH_BANK (HAL_GPIO_PORT_NUM*HAL_GPIO_PIN_NUM_EACH_PORT)

#define HAL_GPIO_PIN_TO_BANK(pin) \
    ((pin)/HAL_GPIO_PIN_NUM_EACH_BANK)

#define HAL_GPIO_PIN_TO_PORT(pin) \
    (((pin)%HAL_GPIO_PIN_NUM_EACH_BANK)/HAL_GPIO_PIN_NUM_EACH_PORT)

#if defined(HAL_GPIO_BANK0_PIN_NUM) && (HAL_GPIO_BANK0_PIN_NUM > HAL_GPIO_PIN_NUM_EACH_BANK)
#error "Bad HAL_GPIO_BANK0_PIN_NUM"
#endif
#ifdef GPIO1_BASE
#if defined(HAL_GPIO_BANK1_PIN_NUM) && (HAL_GPIO_BANK1_PIN_NUM > HAL_GPIO_PIN_NUM_EACH_BANK)
#error "Bad HAL_GPIO_BANK1_PIN_NUM"
#endif
#endif
#ifdef GPIO2_BASE
#if defined(HAL_GPIO_BANK2_PIN_NUM) && (HAL_GPIO_BANK2_PIN_NUM > HAL_GPIO_PIN_NUM_EACH_BANK)
#error "Bad HAL_GPIO_BANK2_PIN_NUM"
#endif
#endif
#ifdef GPIO3_BASE
#if defined(HAL_GPIO_BANK3_PIN_NUM) && (HAL_GPIO_BANK3_PIN_NUM > HAL_GPIO_PIN_NUM_EACH_BANK)
#error "Bad HAL_GPIO_BANK3_PIN_NUM"
#endif
#endif
#ifdef GPIO4_BASE
#if defined(HAL_GPIO_BANK4_PIN_NUM) && (HAL_GPIO_BANK4_PIN_NUM > HAL_GPIO_PIN_NUM_EACH_BANK)
#error "Bad HAL_GPIO_BANK4_PIN_NUM"
#endif
#endif

static struct GPIO_BANK_T * const gpio_bank[] = {
    (struct GPIO_BANK_T *)GPIO_BASE,
#ifdef GPIO1_BASE
    (struct GPIO_BANK_T *)GPIO1_BASE,
#endif
#ifdef GPIO2_BASE
    (struct GPIO_BANK_T *)GPIO2_BASE,
#endif
#ifdef GPIO3_BASE
    (struct GPIO_BANK_T *)GPIO3_BASE,
#endif
#ifdef GPIO4_BASE
    (struct GPIO_BANK_T *)GPIO4_BASE,
#endif
};

static const uint8_t gpio_num[HAL_GPIO_BANK_NUM] = {
#ifdef HAL_GPIO_BANK0_PIN_NUM
    HAL_GPIO_BANK0_PIN_NUM,
#else
    HAL_GPIO_PIN_NUM_EACH_BANK,
#endif
#ifdef GPIO1_BASE
#ifdef HAL_GPIO_BANK1_PIN_NUM
    HAL_GPIO_BANK1_PIN_NUM,
#else
    HAL_GPIO_PIN_NUM_EACH_BANK,
#endif
#endif
#ifdef GPIO2_BASE
#ifdef HAL_GPIO_BANK2_PIN_NUM
    HAL_GPIO_BANK2_PIN_NUM,
#else
    HAL_GPIO_PIN_NUM_EACH_BANK,
#endif
#endif
#ifdef GPIO3_BASE
#ifdef HAL_GPIO_BANK3_PIN_NUM
    HAL_GPIO_BANK3_PIN_NUM,
#else
    HAL_GPIO_PIN_NUM_EACH_BANK,
#endif
#endif
#ifdef GPIO4_BASE
#ifdef HAL_GPIO_BANK4_PIN_NUM
    HAL_GPIO_BANK4_PIN_NUM,
#else
    HAL_GPIO_PIN_NUM_EACH_BANK,
#endif
#endif
};

#if defined(CHIP_BEST2000) && defined(PMU_HAS_LED_PIN)
static struct GPIO_BANK_T * const led_gpio_bank[1] = { (struct GPIO_BANK_T *)AON_GPIO1_BASE, };
#endif

static const char * const err_bad_pin = "Bad GPIO pin %u (should < %u)";

typedef void (* _HAL_GPIO_IRQ_HANDLER)(void);

static HAL_GPIO_PIN_IRQ_HANDLER gpio_irq_handler[HAL_GPIO_PIN_NUM] = {0};

static enum HAL_GPIO_PIN_T hal_gpio_bank_offset_to_pin(uint32_t bank, uint32_t offset)
{
    uint32_t p;

    ASSERT(bank < HAL_GPIO_BANK_NUM, "%s: Bad bank: %u", __func__, bank);
    ASSERT(offset < gpio_num[bank], "%s: Bad offset @ bank %u: %u (should < %u)", __func__, bank, offset, gpio_num[bank]);

    p = 0;
    for (uint32_t i = 0; i < bank; i++) {
        p += gpio_num[i];
    }
    return (enum HAL_GPIO_PIN_T)(p + offset);
}

enum HAL_GPIO_DIR_T hal_gpio_pin_get_dir(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0, port = 0;
    enum HAL_GPIO_DIR_T dir = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0)
    {
    }
#ifdef PMU_HAS_LED_PIN
    else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM))
    {
        return pmu_led_get_direction(pin);
    }
#endif
    else
    {
        bank = HAL_GPIO_PIN_TO_BANK(pin);
        port = HAL_GPIO_PIN_TO_PORT(pin);
        pin_offset = pin%HAL_GPIO_PIN_NUM_EACH_PORT;

        if (gpio_bank[bank]->port[port].GPIO_DDR & (0x1<<pin_offset))
            dir = HAL_GPIO_DIR_OUT;
        else
            dir = HAL_GPIO_DIR_IN;
    }

    return dir;
}

void hal_gpio_pin_dir(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir)
{
    int pin_offset = 0;
    int bank = 0, port = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0)
    {
    }
#ifdef PMU_HAS_LED_PIN
    else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM))
    {
        pmu_led_set_direction(pin, dir);
    }
#endif
    else
    {
        bank = HAL_GPIO_PIN_TO_BANK(pin);
        port = HAL_GPIO_PIN_TO_PORT(pin);
        pin_offset = pin%HAL_GPIO_PIN_NUM_EACH_PORT;

        if(dir == HAL_GPIO_DIR_OUT)
            gpio_bank[bank]->port[port].GPIO_DDR |= 0x1<<pin_offset;
        else
            gpio_bank[bank]->port[port].GPIO_DDR &= ~(0x1<<pin_offset);
    }
}

void hal_gpio_pin_set_dir(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir, uint8_t val_for_out)
{
    if (dir == HAL_GPIO_DIR_OUT) {
        if(val_for_out) {
            hal_gpio_pin_set(pin);
        } else {
            hal_gpio_pin_clr(pin);
        }
    }
    hal_gpio_pin_dir(pin, dir);
}

void hal_gpio_pin_set(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0, port = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0)
    {
    }
#ifdef PMU_HAS_LED_PIN
    else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM))
    {
#ifdef CHIP_BEST2000
        pin_offset = pin - HAL_GPIO_PIN_LED1 + 6;

        led_gpio_bank[0]->port[0].GPIO_DR |= 0x1<<pin_offset;
#else
        pmu_led_set_value(pin, 1);
#endif
    }
#endif
    else
    {
        bank = HAL_GPIO_PIN_TO_BANK(pin);
        port = HAL_GPIO_PIN_TO_PORT(pin);
        pin_offset = pin%HAL_GPIO_PIN_NUM_EACH_PORT;

        gpio_bank[bank]->port[port].GPIO_DR |= 0x1<<pin_offset;
    }
}

void hal_gpio_pin_clr(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0, port = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0)
    {
    }
#ifdef PMU_HAS_LED_PIN
    else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM))
    {
#ifdef CHIP_BEST2000
        pin_offset = pin - HAL_GPIO_PIN_LED1 + 6;

        led_gpio_bank[0]->port[0].GPIO_DR &= ~(0x1<<pin_offset);
#else
        pmu_led_set_value(pin, 0);
#endif
    }
#endif
    else
    {
        bank = HAL_GPIO_PIN_TO_BANK(pin);
        port = HAL_GPIO_PIN_TO_PORT(pin);
        pin_offset = pin%HAL_GPIO_PIN_NUM_EACH_PORT;

        gpio_bank[bank]->port[port].GPIO_DR &= ~(0x1<<pin_offset);
    }
}

uint32_t hal_gpio_pin_get_val(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0, port = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0)
    {
    }
#ifdef PMU_HAS_LED_PIN
    else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM))
    {
#ifdef CHIP_BEST2000
        pin_offset = pin - HAL_GPIO_PIN_LED1 + 6;

        return (((led_gpio_bank[0]->GPIO_EXT_PORT[0]) & (0x1<<pin_offset)) >> pin_offset);
#else
        return pmu_led_get_value(pin);
#endif
    }
#endif
    else
    {
        bank = HAL_GPIO_PIN_TO_BANK(pin);
        port = HAL_GPIO_PIN_TO_PORT(pin);
        pin_offset = pin%HAL_GPIO_PIN_NUM_EACH_PORT;

        /* when as input : read back outside signal value */
        /* when as output: read back DR register value ,same as read back DR register */

        return (((gpio_bank[bank]->GPIO_EXT_PORT[port]) & (0x1<<pin_offset)) >> pin_offset);
    }
}

#ifdef PSC_GPIO_IRQ_CTRL
static void _hal_gpio_irq_handler(void)
{
    uint32_t raw_status[HAL_GPIO_BANK_NUM];
    uint32_t bank = 0, pin_offset = 0;
    uint32_t cnt = 0;
    enum HAL_GPIO_PIN_T pin;

    cnt = hal_psc_gpio_irq_get_status(raw_status, ARRAY_SIZE(raw_status));

    for (bank = cnt; bank < HAL_GPIO_BANK_NUM; bank++) {
        if (gpio_bank[bank]) {
            raw_status[bank] = gpio_bank[bank]->GPIO_RAW_INTSTATUS;
        } else {
            raw_status[bank] = 0;
        }
    }

    for (bank = 0; bank < HAL_GPIO_BANK_NUM; bank++) {
        if (!gpio_bank[bank]) {
            continue;
        }

        gpio_bank[bank]->GPIO_PORTA_EOI = raw_status[bank];

        while (raw_status[bank]) {
            pin_offset = get_lsb_pos(raw_status[bank]);
            pin = hal_gpio_bank_offset_to_pin(bank, pin_offset);
            if (gpio_irq_handler[pin]) {
                gpio_irq_handler[pin](pin);
            }
            raw_status[bank] &= ~(0x1 << pin_offset);
        }
    }
}
#else
static void _hal_gpio_bank_irq_handler(uint32_t bank)
{
    uint32_t raw_status;
    uint32_t pin_offset = 0;
    enum HAL_GPIO_PIN_T pin;

    if (!gpio_bank[bank]) {
        return;
    }

    raw_status = gpio_bank[bank]->GPIO_RAW_INTSTATUS;

#ifdef CMU_GPIO_IRQ_OVERRIDE
    raw_status = hal_cmu_gpio_irq_override(raw_status);
#endif

    if (raw_status == 0) {
        return;
    }

    gpio_bank[bank]->GPIO_PORTA_EOI = raw_status;

    while (raw_status) {
        pin_offset = get_lsb_pos(raw_status);
        pin = hal_gpio_bank_offset_to_pin(bank, pin_offset);
        if (gpio_irq_handler[pin]) {
            gpio_irq_handler[pin](pin);
        }
        raw_status &= ~(0x1 << pin_offset);
    }
}

static const IRQn_Type gpio_irq_n[HAL_GPIO_BANK_NUM];

static inline void _hal_gpio_loop_bank_irq_handler(const IRQn_Type irq)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(gpio_irq_n); i++) {
        if (irq == gpio_irq_n[i]) {
            _hal_gpio_bank_irq_handler(i);
        }
    }
}

static void _hal_gpio0_irq_handler(void)
{
    _hal_gpio_loop_bank_irq_handler(GPIO_IRQn);
}

#ifdef GPIO1_BASE
static void _hal_gpio1_irq_handler(void)
{
    _hal_gpio_loop_bank_irq_handler(GPIO1_IRQn);
}
#endif

#ifdef GPIO2_BASE
static void _hal_gpio2_irq_handler(void)
{
    _hal_gpio_loop_bank_irq_handler(GPIO2_IRQn);
}
#endif

#ifdef GPIO3_BASE
static void _hal_gpio3_irq_handler(void)
{
    _hal_gpio_loop_bank_irq_handler(GPIO3_IRQn);
}
#endif

#ifdef GPIO4_BASE
static void _hal_gpio4_irq_handler(void)
{
    _hal_gpio_loop_bank_irq_handler(GPIO4_IRQn);
}
#endif

static const uint32_t gpio_irq_entry[HAL_GPIO_BANK_NUM] = {
    (uint32_t)_hal_gpio0_irq_handler,
#ifdef GPIO1_BASE
    (uint32_t)_hal_gpio1_irq_handler,
#endif
#ifdef GPIO2_BASE
    (uint32_t)_hal_gpio2_irq_handler,
#endif
#ifdef GPIO3_BASE
    (uint32_t)_hal_gpio3_irq_handler,
#endif
#ifdef GPIO4_BASE
    (uint32_t)_hal_gpio4_irq_handler,
#endif
};

static const IRQn_Type gpio_irq_n[HAL_GPIO_BANK_NUM] = {
    GPIO_IRQn,
#ifdef GPIO1_BASE
    GPIO1_IRQn,
#endif
#ifdef GPIO2_BASE
    GPIO2_IRQn,
#endif
#ifdef GPIO3_BASE
    GPIO3_IRQn,
#endif
#ifdef GPIO4_BASE
    GPIO4_IRQn,
#endif
};
#endif

uint32_t hal_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg)
{
    int pin_offset = 0;
    int bank = 0, port = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#if defined(PMU_HAS_LED_PIN) && defined(PMU_HAS_LED_IRQ)
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0)
    {
    }
#if defined(PMU_HAS_LED_PIN) && defined(PMU_HAS_LED_IRQ)
    else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM))
    {
        return pmu_gpio_setup_irq(pin, cfg);
    }
#endif
    else
    {
        bank = HAL_GPIO_PIN_TO_BANK(pin);
        port = HAL_GPIO_PIN_TO_PORT(pin);
        pin_offset = pin%HAL_GPIO_PIN_NUM_EACH_PORT;

        /* only port A support irq */
        if (port != 0)
            return 0;

        if (cfg->irq_enable) {
            gpio_bank[bank]->GPIO_INTMASK |= (0x1<<pin_offset);

            if (cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE)
                gpio_bank[bank]->GPIO_INTTYPE_LEVEL |= 0x1<<pin_offset;
            else
                gpio_bank[bank]->GPIO_INTTYPE_LEVEL &= ~(0x1<<pin_offset);

            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_HIGH_RISING)
                gpio_bank[bank]->GPIO_INT_POLARITY |= 0x1<<pin_offset;
            else
                gpio_bank[bank]->GPIO_INT_POLARITY &= ~(0x1<<pin_offset);

            // If debounce is disabled, 4 APB clocks are needed before enabling INT
            // to avoid the (first) fake edge IRQ
#ifdef GPIO_FIRST_EDGE_IRQ_WORKAROUND
            if (cfg->irq_debounce && cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
                // Make sure the debounce input sees the newest value (in 32K clock)
                hal_sys_timer_delay_us(50);
            }
#endif

            if (cfg->irq_debounce)
                gpio_bank[bank]->GPIO_DEBOUNCE |= 0x1<<pin_offset;
            else
                gpio_bank[bank]->GPIO_DEBOUNCE &= ~(0x1<<pin_offset);

            gpio_irq_handler[pin] = cfg->irq_handler;

#ifdef PSC_GPIO_IRQ_CTRL
            NVIC_SetVector(GPIO_IRQn, (uint32_t)_hal_gpio_irq_handler);
            NVIC_SetPriority(GPIO_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_EnableIRQ(GPIO_IRQn);
            hal_psc_gpio_irq_enable(pin);
#else
            NVIC_SetVector(gpio_irq_n[bank], gpio_irq_entry[bank]);
            NVIC_SetPriority(gpio_irq_n[bank], IRQ_PRIORITY_NORMAL);
            NVIC_EnableIRQ(gpio_irq_n[bank]);
#endif

            gpio_bank[bank]->GPIO_PORTA_EOI = (0x1 << pin_offset);
            gpio_bank[bank]->GPIO_INTMASK &= ~(0x1<<pin_offset);
            gpio_bank[bank]->GPIO_INTEN |= 0x1<<pin_offset;
        }
        else {
#ifdef PSC_GPIO_IRQ_CTRL
            hal_psc_gpio_irq_disable(pin);
#endif
            gpio_bank[bank]->GPIO_INTMASK |= (0x1<<pin_offset);
            gpio_bank[bank]->GPIO_INTEN &= ~(0x1<<pin_offset);
            gpio_irq_handler[pin] = 0;
        }
    }
    return 0;
}

uint32_t hal_gpio_get_max_pin_num(void)
{
    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
    return max_pin;
}

#endif

#endif
