/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#if (CHIP_GPIO_VER >= 2)

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
#include "reg_gpio_v2.h"

#define HAL_GPIO_PIN_NUM_EACH_BANK          (32)

#define HAL_GPIO_BANK_NUM                   ARRAY_SIZE(gpio_bank)

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

static HAL_GPIO_PIN_IRQ_HANDLER gpio_irq_handler[HAL_GPIO_PIN_NUM];

static const char * const err_bad_pin = "Bad GPIO pin %u (should < %u)";

#ifdef GPIO_BOTH_EDGE_IRQ
#ifndef GPIO_HAS_BOTH_EDGE_IRQ
#error "Both edge IRQ not supported"
#endif
#ifdef PSC_GPIO_IRQ_CTRL
static enum HAL_GPIO_IRQ_TYPE_T gpio_both_edge_irq_type;
#else
static enum HAL_GPIO_IRQ_TYPE_T gpio_both_edge_irq_type[HAL_GPIO_BANK_NUM];
#endif
#endif

static uint32_t hal_gpio_pin_to_bank(enum HAL_GPIO_PIN_T pin)
{
    uint32_t p = pin;

    for (uint32_t i = 0; i < HAL_GPIO_BANK_NUM; i++) {
        if (p < gpio_num[i]) {
            ASSERT(gpio_bank[i], "%s: Bad bank %u for gpio pin: %u", __func__, i, pin);
            return i;
        }
        p -= gpio_num[i];
    }
    ASSERT(false, "%s: Bad gpio pin: %u", __func__, pin);
    return 0;
}

static uint32_t hal_gpio_pin_to_offset(enum HAL_GPIO_PIN_T pin)
{
    uint32_t p = pin;

    for (uint32_t i = 0; i < HAL_GPIO_BANK_NUM; i++) {
        if (p < gpio_num[i]) {
            return p;
        }
        p -= gpio_num[i];
    }
    ASSERT(false, "%s: Bad gpio pin: %u", __func__, pin);
    return 0;
}

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
    int bank = 0;
    enum HAL_GPIO_DIR_T dir = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        return pmu_led_get_direction(pin);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        return pmu_gpio_pin_get_dir(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        if (gpio_bank[bank]->GPIO_DDR_SET & (0x1 << pin_offset)) {
            dir = HAL_GPIO_DIR_OUT;
        } else {
            dir = HAL_GPIO_DIR_IN;
        }
    }

    return dir;
}

void hal_gpio_pin_dir(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir)
{
    int pin_offset = 0;
    int bank = 0;
    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        pmu_led_set_direction(pin, dir);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        pmu_gpio_pin_dir(pin, dir);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        if(dir == HAL_GPIO_DIR_OUT) {
            gpio_bank[bank]->GPIO_DDR_SET = (0x1 << pin_offset);
        } else {
            gpio_bank[bank]->GPIO_DDR_CLR = (0x1 << pin_offset);
        }
    }
}

void hal_gpio_pin_set_dir(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir, uint8_t val_for_out)
{
    if (dir == HAL_GPIO_DIR_OUT) {
        if (val_for_out) {
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
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        pmu_led_set_value(pin, 1);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        pmu_gpio_pin_set(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        gpio_bank[bank]->GPIO_DR_SET = (0x1 << pin_offset);
    }
}

void hal_gpio_pin_clr(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        pmu_led_set_value(pin, 0);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        pmu_gpio_pin_clr(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        gpio_bank[bank]->GPIO_DR_CLR = (0x1 << pin_offset);
    }
}

uint32_t hal_gpio_pin_get_val(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
        return pmu_led_get_value(pin);
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        return pmu_gpio_pin_get_input_val(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        return !!(gpio_bank[bank]->GPIO_EXT_PORT & (0x1 << pin_offset));
    }
}

uint8_t hal_gpio_pin_get_out_val(enum HAL_GPIO_PIN_T pin)
{
    int pin_offset = 0;
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#ifdef PMU_HAS_LED_PIN
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
        return pmu_gpio_pin_get_output_val(pin);
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        return !!(gpio_bank[bank]->GPIO_DR_SET  & (0x1 << pin_offset));
    }
}

#ifdef PSC_GPIO_IRQ_CTRL
static void _hal_gpio_irq_handler(void)
{
    uint32_t raw_status[HAL_GPIO_BANK_NUM];
#ifdef GPIO_BOTH_EDGE_IRQ
    uint32_t raw_status_f[HAL_GPIO_BANK_NUM];
#endif
    uint32_t bank = 0, pin_offset = 0;
    uint32_t cnt = 0;
    enum HAL_GPIO_PIN_T pin;

    cnt = hal_psc_gpio_irq_get_status(raw_status, ARRAY_SIZE(raw_status));

#ifdef GPIO_BOTH_EDGE_IRQ
    for (bank = 0; bank < cnt; bank++) {
        raw_status_f[bank] = raw_status[bank] & gpio_bank[bank]->GPIO_RAW_INTSTATUS_F;
        raw_status[bank] &= gpio_bank[bank]->GPIO_RAW_INTSTATUS;
    }
#endif

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
#ifdef GPIO_BOTH_EDGE_IRQ
        gpio_bank[bank]->GPIO_PORTA_EOI_F = raw_status_f[bank];
#endif

        while (raw_status[bank]) {
            pin_offset = get_lsb_pos(raw_status[bank]);
            pin = hal_gpio_bank_offset_to_pin(bank, pin_offset);
            if (gpio_irq_handler[pin]) {
#ifdef GPIO_BOTH_EDGE_IRQ
                gpio_both_edge_irq_type = HAL_GPIO_BOTH_EDGE_IRQ_TYPE_RISING;
#endif
                gpio_irq_handler[pin](pin);
            }
            raw_status[bank] &= ~(0x1 << pin_offset);
        }
#ifdef GPIO_BOTH_EDGE_IRQ
        while (raw_status_f[bank]) {
            pin_offset = get_lsb_pos(raw_status_f[bank]);
            pin = hal_gpio_bank_offset_to_pin(bank, pin_offset);
            if (gpio_irq_handler[pin]) {
#ifdef GPIO_BOTH_EDGE_IRQ
                gpio_both_edge_irq_type = HAL_GPIO_BOTH_EDGE_IRQ_TYPE_FALLING;
#endif
                gpio_irq_handler[pin](pin);
            }
            raw_status_f[bank] &= ~(0x1 << pin_offset);
        }
#endif
    }
}
#else
static void _hal_gpio_bank_irq_handler(uint32_t bank)
{
    uint32_t raw_status;
#ifdef GPIO_BOTH_EDGE_IRQ
    uint32_t raw_status_f;
#endif
    uint32_t pin_offset = 0;
    enum HAL_GPIO_PIN_T pin;

    if (!gpio_bank[bank]) {
        return;
    }

    raw_status = gpio_bank[bank]->GPIO_RAW_INTSTATUS;
#ifdef GPIO_BOTH_EDGE_IRQ
    raw_status_f = gpio_bank[bank]->GPIO_RAW_INTSTATUS_F;
#endif

    gpio_bank[bank]->GPIO_PORTA_EOI = raw_status;
#ifdef GPIO_BOTH_EDGE_IRQ
    gpio_bank[bank]->GPIO_PORTA_EOI_F = raw_status_f;
#endif

    while (raw_status) {
        pin_offset = get_lsb_pos(raw_status);
        pin = hal_gpio_bank_offset_to_pin(bank, pin_offset);
        if (gpio_irq_handler[pin]) {
#ifdef GPIO_BOTH_EDGE_IRQ
            gpio_both_edge_irq_type[bank] = HAL_GPIO_BOTH_EDGE_IRQ_TYPE_RISING;
#endif
            gpio_irq_handler[pin](pin);
        }
        raw_status &= ~(0x1 << pin_offset);
    }
#ifdef GPIO_BOTH_EDGE_IRQ
    while (raw_status_f) {
        pin_offset = get_lsb_pos(raw_status_f);
        pin = hal_gpio_bank_offset_to_pin(bank, pin_offset);
        if (gpio_irq_handler[pin]) {
            gpio_both_edge_irq_type[bank] = HAL_GPIO_BOTH_EDGE_IRQ_TYPE_FALLING;
            gpio_irq_handler[pin](pin);
        }
        raw_status_f &= ~(0x1 << pin_offset);
    }
#endif
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
    int bank = 0;

    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;
#if defined(PMU_HAS_LED_PIN) && defined(PMU_HAS_LED_IRQ)
    max_pin = HAL_GPIO_PIN_LED_NUM;
#endif
#if defined(PMU_HAS_GPIO_PIN) && defined(PMU_HAS_GPIO_IRQ)
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

    if (0) {
#ifdef PMU_HAS_LED_PIN
    } else if ((HAL_GPIO_PIN_LED1 <= pin) && (pin < HAL_GPIO_PIN_LED_NUM)) {
#ifdef PMU_HAS_LED_IRQ
        return pmu_gpio_setup_irq(pin, cfg);
#else
        return 1;
#endif
#endif
#ifdef PMU_HAS_GPIO_PIN
    } else if (pin >= HAL_GPIO_PIN_PMU_P0_0 && pin < HAL_GPIO_PIN_PMU_NUM) {
#ifdef PMU_HAS_GPIO_IRQ
        return pmu_gpio_setup_irq(pin, cfg);
#else
        return 1;
#endif
#endif
    } else {
        bank = hal_gpio_pin_to_bank(pin);
        pin_offset = hal_gpio_pin_to_offset(pin);

        if (cfg->irq_enable) {
#ifndef GPIO_BOTH_EDGE_IRQ
            ASSERT(cfg->irq_polarity != HAL_GPIO_IRQ_POLARITY_EDGE_BOTH,\
                "%s: Not support edge both: pin=%u", __func__, pin);
#endif

            gpio_bank[bank]->GPIO_INTMASK_SET = (0x1 << pin_offset);
#ifdef GPIO_BOTH_EDGE_IRQ
            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_EDGE_BOTH) {
                ASSERT(cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,\
                    "%s: Bad irq type! pin=%u", __func__, pin);
                gpio_bank[bank]->GPIO_INTMASK_F_SET = (0x1 << pin_offset);
            }
#endif

            if (cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
                gpio_bank[bank]->GPIO_INTTYPE_LEVEL_SET = (0x1 << pin_offset);
            } else {
                gpio_bank[bank]->GPIO_INTTYPE_LEVEL_CLR = (0x1 << pin_offset);
            }

            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_HIGH_RISING || cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_EDGE_BOTH) {
                gpio_bank[bank]->GPIO_INT_POLARITY_SET = (0x1 << pin_offset);
            } else {
                gpio_bank[bank]->GPIO_INT_POLARITY_CLR = (0x1 << pin_offset);
            }
            // If debounce is disabled, 4 APB clocks are needed before enabling INT
            // to avoid the (first) fake edge IRQ

            if (cfg->irq_debounce) {
#ifdef GPIO_FIRST_EDGE_IRQ_WORKAROUND
                if (cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
                    // Make sure the debounce input sees the newest value (in 32K clock)
                    hal_sys_timer_delay_us(50);
                }
#endif
                gpio_bank[bank]->GPIO_DEBOUNCE_SET = (0x1 << pin_offset);
            } else {
                gpio_bank[bank]->GPIO_DEBOUNCE_CLR = (0x1 << pin_offset);
            }

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
            gpio_bank[bank]->GPIO_INTMASK_CLR = (0x1 << pin_offset);
            gpio_bank[bank]->GPIO_INTEN_SET = (0x1 << pin_offset);
#ifdef GPIO_BOTH_EDGE_IRQ
            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_EDGE_BOTH) {
                gpio_bank[bank]->GPIO_PORTA_EOI_F = (0x1 << pin_offset);
                gpio_bank[bank]->GPIO_INTMASK_F_CLR = (0x1 << pin_offset);
                gpio_bank[bank]->GPIO_INTEN_F_SET = (0x1 << pin_offset);
            }
#endif
        } else {
#ifdef PSC_GPIO_IRQ_CTRL
            hal_psc_gpio_irq_disable(pin);
#endif

            gpio_bank[bank]->GPIO_INTMASK_SET = (0x1 << pin_offset);
            gpio_bank[bank]->GPIO_INTEN_CLR = (0x1 << pin_offset);
#ifdef GPIO_BOTH_EDGE_IRQ
            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_EDGE_BOTH) {
                gpio_bank[bank]->GPIO_INTMASK_F_SET = (0x1 << pin_offset);
                gpio_bank[bank]->GPIO_INTEN_F_CLR = (0x1 << pin_offset);
            }
#endif
            gpio_irq_handler[pin] = NULL;
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
#ifdef PMU_HAS_GPIO_PIN
    max_pin = HAL_GPIO_PIN_PMU_NUM;
#endif
    return max_pin;
}

enum HAL_GPIO_BOTH_EDGE_IRQ_TYPE_T hal_gpio_both_edge_irq_type_get(enum HAL_GPIO_PIN_T pin)
{
    enum HAL_GPIO_PIN_T max_pin = HAL_GPIO_PIN_NUM;

    ASSERT(pin < max_pin, err_bad_pin, pin, max_pin);

#ifdef GPIO_BOTH_EDGE_IRQ
#ifdef PSC_GPIO_IRQ_CTRL
    return gpio_both_edge_irq_type;
#else
    int bank = hal_gpio_pin_to_bank(pin);

    return gpio_both_edge_irq_type[bank];
#endif
#else
    return HAL_GPIO_BOTH_EDGE_IRQ_TYPE_NONE;
#endif
}

#endif

#endif
