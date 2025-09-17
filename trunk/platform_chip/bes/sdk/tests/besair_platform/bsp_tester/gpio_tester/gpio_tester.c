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
#include "gpio_tester.h"

#define PIN(n)              (enum HAL_GPIO_PIN_T)(((n) / 10) * 8 + ((n) % 10))
#define TRIGGERED_PIN       (enum HAL_GPIO_PIN_T)15
#define TOGGLE_DELAY_TIME   500

static enum HAL_GPIO_PIN_T pin;

static const char *const cmd[] = {
    "output_low",
    "output_high",
    "input",
    "toggle",
    "irq_low",
    "irq_high",
    "irq_fall",
    "irq_rise",
    "irq_disable"
};

enum GPIO_PIN_CMD {
    GPIO_PIN_OUTPUT_LOW,
    GPIO_PIN_OUTPUT_HIGH,
    GPIO_PIN_INPUT,
    GPIO_PIN_TOGGLE,
    GPIO_PIN_IRQ_LOW,
    GPIO_PIN_IRQ_HIGH,
    GPIO_PIN_IRQ_FALL,
    GPIO_PIN_IRQ_RISE,
    GPIO_PIN_IRQ_DISABLE,

    GPIO_PIN_CMD_NUM
};

static void gpio_tester_set_output_low (enum HAL_GPIO_PIN_T pin)
{
    CMD_TRACE("output pin p%02d is set to LOW",  pin);
    hal_gpio_pin_set_dir(PIN(pin), HAL_GPIO_DIR_OUT, GPIO_PIN_OUTPUT_LOW );
}

static void gpio_tester_set_output_high(enum HAL_GPIO_PIN_T pin)
{
    CMD_TRACE("output pin p%02d is set to HIGH", pin);
    hal_gpio_pin_set_dir(PIN(pin), HAL_GPIO_DIR_OUT, GPIO_PIN_OUTPUT_HIGH);
}

static void gpio_tester_set_input      (enum HAL_GPIO_PIN_T pin)
{
    CMD_TRACE("input pin p%02d is set to Hi-Z",  pin);
    hal_gpio_pin_set_dir(PIN(pin), HAL_GPIO_DIR_IN, GPIO_PIN_OUTPUT_LOW  );
    hal_gpio_pin_get_val(PIN(pin)) ? CMD_TRACE("input pin p%02d value:\tHIGH", pin) :
                                     CMD_TRACE("input pin p%02d value:\tLOW" , pin) ;
}

static void gpio_tester_set_toggle     (enum HAL_GPIO_PIN_T pin)
{
    CMD_TRACE("toggling pin p%02d every %d ms", pin, TOGGLE_DELAY_TIME);
    hal_gpio_pin_set_dir(PIN(pin), HAL_GPIO_DIR_OUT, GPIO_PIN_OUTPUT_LOW );
    for(uint8_t count = 10; count; count--) {
        hal_gpio_pin_set(PIN(pin));
        osDelay(TOGGLE_DELAY_TIME);
        hal_gpio_pin_clr(PIN(pin));
        osDelay(TOGGLE_DELAY_TIME);
    }
}

static void gpio_tester_pin_init(enum HAL_GPIO_PIN_T pin)
{
    /* gpio pin related attributions setting */
    const struct HAL_IOMUX_PIN_FUNCTION_MAP pinAttr = {
        PIN(pin),
        HAL_IOMUX_FUNC_GPIO,
        HAL_IOMUX_PIN_VOLTAGE_VIO,
        HAL_IOMUX_PIN_PULLUP_ENABLE
    };

    /* gpio pin initialization */
    hal_iomux_init(&pinAttr, 1);
}

static void gpio_irq_handler           (enum HAL_GPIO_PIN_T pin)
{
    CMD_TRACE("pin p%02d is triggered", TRIGGERED_PIN);
    gpio_tester_pin_init(TRIGGERED_PIN);
    gpio_tester_set_output_high(TRIGGERED_PIN);
}

static void gpio_tester_set_irq        (enum HAL_GPIO_PIN_T pin, uint8_t op)
{
    /* gpio pin irq related attributions setting */
    struct HAL_GPIO_IRQ_CFG_T gpioAttr = {
        .irq_enable         = true,
        .irq_debounce       = false,
        .irq_type           = (op <= GPIO_PIN_IRQ_HIGH) ?
                                HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE :
                                HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
        .irq_polarity       = gpioAttr.irq_type ?
                                (enum HAL_GPIO_IRQ_POLARITY_T)(op - GPIO_PIN_IRQ_FALL) :
                                (enum HAL_GPIO_IRQ_POLARITY_T)(op - GPIO_PIN_IRQ_LOW),
        .irq_handler        = gpio_irq_handler,
    };

    char *type = gpioAttr.irq_type ? "edge" : "level";
    char *polarity = gpioAttr.irq_type ?
                        (gpioAttr.irq_polarity ? "rising" : "falling") :
                        (gpioAttr.irq_polarity ? "high"   : "low");
    CMD_TRACE("pin p%02d is set as interrupt port (%s, %s)", pin, type, polarity);

    /* gpio pin irq initialization */
    hal_gpio_setup_irq(PIN(pin), &gpioAttr);
}

static void gpio_tester_clr_irq        (enum HAL_GPIO_PIN_T pin)
{
    /* gpio pin irq related attributions setting */
    struct HAL_GPIO_IRQ_CFG_T gpioAttr = {
        .irq_enable         = false,
        .irq_handler        = gpio_irq_handler,
    };

    CMD_TRACE("pin p%02d as interrupt port is disabled", pin);

    /* gpio pin irq initialization */
    hal_gpio_setup_irq(PIN(pin), &gpioAttr);
}

void gpio_tester(const char **params)
{
    bool flag = false;

    ASSIGN_VAL(pin, params[1]);

    PARAM_CHK(PIN_CHK(pin));

    gpio_tester_pin_init(pin);

    for (uint8_t op = 0; op < GPIO_PIN_CMD_NUM; op++) {
        if (!strncmp(cmd[op], params[0], strlen(cmd[op]))) {
            flag = true;
            switch (op) {
                case GPIO_PIN_OUTPUT_LOW:
                    gpio_tester_set_output_low (pin);
                    break;
                case GPIO_PIN_OUTPUT_HIGH:
                    gpio_tester_set_output_high(pin);
                    break;
                case GPIO_PIN_INPUT:
                    gpio_tester_set_input      (pin);
                    break;
                case GPIO_PIN_TOGGLE:
                    gpio_tester_set_toggle     (pin);
                    break;
                case GPIO_PIN_IRQ_LOW :
                case GPIO_PIN_IRQ_HIGH:
                case GPIO_PIN_IRQ_FALL:
                case GPIO_PIN_IRQ_RISE:
                    gpio_tester_set_irq        (pin, op);
                    break;
                case GPIO_PIN_IRQ_DISABLE:
                    gpio_tester_clr_irq        (pin);
                    break;
            }
            break;
        }
    }
    PARAM_CHK(flag);
}