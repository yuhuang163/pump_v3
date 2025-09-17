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
#ifdef UTILS_ESHELL_EN
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "cmsis_os.h"
#include "eshell.h"
#include "app_eshell.h"

#define PIN_TO_NUM(pin) (pin / 8 * 10 + pin % 8)
#define NUM_TO_PIN(num) (num / 10 * 8 + num % 8)

void test_gpio_set(enum HAL_GPIO_PIN_T pin, uint32_t timeout)
{
    uint32_t i = 0;
    uint32_t count = timeout*2;
    bool hi = 1;
    uint8_t val = 0;
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {0, HAL_IOMUX_FUNC_AS_GPIO,
        HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };

    pinmux[0].pin = pin;

    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(pin, HAL_GPIO_DIR_OUT, 0);

    while (i++ < count) {
        if (hi) {
            hal_gpio_pin_set(pin);
            val = hal_gpio_pin_get_val(pin);
            if (val != 1)
                break;
        } else {
            hal_gpio_pin_clr(pin);
            val = hal_gpio_pin_get_val(pin);
            if (val != 0)
                break;
        }
        eshell_putstring("%d: GPIO %d set to %s\r\n", i, PIN_TO_NUM(pin),
                    hi ? "HIGH" : "LOW");
        hi = !hi;
        osDelay(1000);
    }

    if (i < count)
        eshell_putstring("\r\nGPIO %d set testing is Failed.\r\n", PIN_TO_NUM(pin));
    else
        eshell_putstring("\r\nGPIO %d set testing is Success!\r\n", PIN_TO_NUM(pin));
}

static int gpio_irq_got = 0;
static void test_gpio_irqhandler(enum HAL_GPIO_PIN_T pin)
{
    gpio_irq_got = 1;
    eshell_putstring("GPIO %d IRQ is handled.\r\n", PIN_TO_NUM(pin));
}

static void gpio_set_irq(enum HAL_GPIO_PIN_T pin,
        enum HAL_GPIO_IRQ_POLARITY_T polarity, uint8_t irq_en)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg = {0,};

    if(irq_en == true) {
        gpiocfg.irq_enable = true;
        gpiocfg.irq_handler = test_gpio_irqhandler;
    }else {
        gpiocfg.irq_enable = false;
        gpiocfg.irq_handler = NULL;
    }
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_polarity = polarity;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;

    hal_gpio_setup_irq(pin, &gpiocfg);
}

#define HIGH_RISING 1
#define LOW_FALLING 2

void test_gpio_irq(enum HAL_GPIO_PIN_T pin, int rising, uint32_t timeout)
{
    uint32_t i = 0;
    uint32_t count = timeout*2;
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {0, HAL_IOMUX_FUNC_AS_GPIO,
        HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    if (rising == HIGH_RISING) {
        pinmux[0].pin = pin;
        pinmux[0].pull_sel = HAL_IOMUX_PIN_PULLDOWN_ENABLE;
        hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
        hal_gpio_pin_set_dir(pin, HAL_GPIO_DIR_IN, 0);

        eshell_putstring("\r\nGPIO %d IRQ (High-Rising) ready to test.\r\n",
                    PIN_TO_NUM(pin));
        gpio_irq_got = 0;
        gpio_set_irq(pin, HAL_GPIO_IRQ_POLARITY_HIGH_RISING, true);

        while (i++ < count && gpio_irq_got == 0) {
            osDelay(500);
        }

        gpio_set_irq(pin, HAL_GPIO_IRQ_POLARITY_HIGH_RISING, false);

        if (gpio_irq_got)
            eshell_putstring("\r\nGPIO %d IRQ (High-Rising) testing is Success!\r\n",
                        PIN_TO_NUM(pin));
        else
            eshell_putstring("\r\nGPIO %d IRQ (High-Rising) testing is %s.\r\n",
                    PIN_TO_NUM(pin),
                    (i >= count) ? "timeout" : "Failed");

    } else {
        pinmux[0].pin = pin;
        pinmux[0].pull_sel = HAL_IOMUX_PIN_PULLUP_ENABLE;
        hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
        hal_gpio_pin_set_dir(pin, HAL_GPIO_DIR_IN, 1);

        eshell_putstring("\r\nGPIO %d IRQ (Low-Falling) ready to test.\r\n",
                    PIN_TO_NUM(pin));
        gpio_irq_got = 0;
        gpio_set_irq(pin, HAL_GPIO_IRQ_POLARITY_LOW_FALLING, true);

        while (i++ < count && gpio_irq_got == 0) {
            osDelay(500);
        }

        gpio_set_irq(pin, HAL_GPIO_IRQ_POLARITY_LOW_FALLING, false);

        if (gpio_irq_got)
            eshell_putstring("\r\nGPIO %d IRQ (Low-Falling) testing is Success!\r\n",
                        PIN_TO_NUM(pin));
        else
            eshell_putstring("\r\nGPIO %d IRQ (Low-Falling) testing is %s.\r\n",
                    PIN_TO_NUM(pin),
                    (i >= count) ? "timeout" : "Failed");
    }
}

static void unitest_usage(void)
{
    eshell_putstring("Usage: \r\n");
    eshell_putstring("  utest_gpio set gpio_num [timeout_secs]\r\n");
    eshell_putstring("  utest_gpio irq irq_gpio_num low|high [timeout_secs]\r\n");
}

static void unitest_gpio(int argc, char *argv[])
{
    uint32_t gpiopin = 0;
    uint32_t gpionum = 0;
    uint32_t timeout = 20;

    if (argc < 3)
        goto usage;

    gpionum = atoi(argv[2]);
    if (gpionum % 10 > 7) {
        eshell_putstring("GPIO number must be x0 ~ x7.\r\n");
        return;
    }

    gpiopin = NUM_TO_PIN(gpionum);
    if (gpiopin > HAL_GPIO_PIN_NUM) {
        eshell_putstring("GPIO %d is invalid!\r\n", gpionum);
        return;
    }

    if (strncmp(argv[1], "set", 3) == 0) {
        if (argc >= 4)
            timeout = atoi(argv[3]);
        test_gpio_set(gpiopin, timeout);
    } else if (strncmp(argv[1], "irq", 3) == 0) {
        uint32_t rising = LOW_FALLING;
        if (argc >= 4) {
            if (strncmp(argv[3], "high", 4) == 0)
                rising = HIGH_RISING;
            else if (strncmp(argv[3], "low", 3) == 0)
                rising = LOW_FALLING;
            else
                goto usage;
        }
        if (argc >= 5)
            timeout = atoi(argv[4]);
        test_gpio_irq(gpiopin, rising, timeout);
    } else
        goto usage;

    return;

usage:
    unitest_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_gpio",
                        "usage: utest_gpio help", unitest_gpio);

#endif
