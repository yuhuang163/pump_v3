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
#ifdef TEST_GPIO
#include "hal_trace.h"
#include "hal_gpio.h"
#include "cmsis_os.h"
#include "pmu_best1600.h"

#define PIN_OUT_1               (HAL_GPIO_PIN_P2_4)
#define PIN_OUT_2               (HAL_GPIO_PIN_P2_5)

#define PIN_IN_1                (HAL_IOMUX_PIN_P1_5)
#define PIN_IN_2                (HAL_IOMUX_PIN_P1_7)
#define PIN_IN_3                (HAL_IOMUX_PIN_P3_0)
#define PIN_IN_4                (HAL_IOMUX_PIN_P4_7)
#define PIN_IN_5                (HAL_IOMUX_PIN_P5_4)
#define PIN_IN_6                (HAL_IOMUX_PIN_P5_5)

#define PMU_GPIO_PIN_1          HAL_GPIO_PIN_PMU_P4_6
#define PMU_GPIO_INPUT_PIN_1    HAL_GPIO_PIN_PMU_P4_3

static void test_pin_out_level(enum HAL_GPIO_PIN_T pin, bool hi)
{
    uint8_t val = 0;
    if(hi) {
        hal_gpio_pin_set(pin);
#ifndef CHIP_SUBSYS_SENS
        pmu_gpio_pin_set(PMU_GPIO_PIN_1);
        val = pmu_gpio_pin_get_val(PMU_GPIO_INPUT_PIN_1);
#endif
    } else {
        hal_gpio_pin_clr(pin);
#ifndef CHIP_SUBSYS_SENS
        pmu_gpio_pin_clr(PMU_GPIO_PIN_1);
        val = pmu_gpio_pin_get_val(PMU_GPIO_INPUT_PIN_1);
#endif
    }
    TRACE(1,"test_pin_out_level %d",hi);
    TRACE(1,"test_pin_get_level %d",val);
}

static void test_gpio_irqhandler(enum HAL_GPIO_PIN_T pin)
{
    TRACE(0, "%s pin:%d", __func__, pin);
}

static void gpio_set_irq(enum HAL_GPIO_PIN_T pin,enum HAL_GPIO_IRQ_POLARITY_T polarity,uint8_t irq_en)
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

#ifndef CHIP_SUBSYS_SENS

static void test_pmu_gpio_irqhandler(enum HAL_IOMUX_PIN_T pin)
{
    TRACE(0, "%s pin:%d", __func__, pin);
    TRACE(0, "%d", pmu_gpio_irq_type_get());
}


static void pmu_gpio_test_set_irq(enum HAL_IOMUX_PIN_T pin,enum HAL_GPIO_IRQ_POLARITY_T polarity,uint8_t irq_en)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg = {0,};

    if(irq_en == true) {
        gpiocfg.irq_enable = true;
        gpiocfg.irq_handler = (HAL_GPIO_PIN_IRQ_HANDLER)test_pmu_gpio_irqhandler;
    }else {
        gpiocfg.irq_enable = false;
        gpiocfg.irq_handler = NULL;
    }
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_polarity = polarity;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;

    pmu_gpio_setup_irq(pin, &gpiocfg);
}

#endif

void test_gpio(void)
{
    uint32_t i = 0;
    TRACE(2,"%s test gpio start.", __FUNCTION__);

    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {PIN_OUT_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {PIN_OUT_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };

    /* input pin table */
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_input_tester[] = {
        {PIN_IN_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {PIN_IN_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#ifndef CHIP_SUBSYS_SENS
        {PIN_IN_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {PIN_IN_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {PIN_IN_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {PIN_IN_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
    };

    //TRACE(2,"%s IOMUX ADDR BASE[0x%08x] GPIO ADDR BASE[0x%08x]", __FUNCTION__,
    //                IOMUX_BASE, GPIO_BASE);

    /* IOMUX initialization. */
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    /* DIR: output. */
    hal_gpio_pin_set_dir(PIN_OUT_1, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(PIN_OUT_2, HAL_GPIO_DIR_OUT, 1);

    hal_iomux_init(gpio_input_tester, ARRAY_SIZE(gpio_input_tester));
#if 1
    for (i=0; i<ARRAY_SIZE(gpio_input_tester); i++){
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)gpio_input_tester[i].pin, HAL_GPIO_DIR_IN, 1);
        gpio_set_irq((enum HAL_GPIO_PIN_T)gpio_input_tester[i].pin,HAL_GPIO_IRQ_POLARITY_HIGH_RISING,true);
    }
#ifndef CHIP_SUBSYS_SENS
    pmu_gpio_pin_set_dir(PMU_GPIO_PIN_1, HAL_GPIO_DIR_OUT, 0);
    pmu_gpio_set_io_pull_select(PMU_GPIO_PIN_1, HAL_IOMUX_PIN_PULLUP_ENABLE);

    pmu_gpio_pin_set_dir(PMU_GPIO_INPUT_PIN_1, HAL_GPIO_DIR_IN, 0);
    pmu_gpio_set_io_pull_select(PMU_GPIO_INPUT_PIN_1, HAL_IOMUX_PIN_PULLUP_ENABLE);
    pmu_gpio_test_set_irq(PMU_GPIO_INPUT_PIN_1,HAL_GPIO_IRQ_POLARITY_EDGE_BOTH,true);
#endif
    bool hi = 0;

    do {
        test_pin_out_level(PIN_OUT_1, (hi = !hi));
        osDelay(500);
    } while(1);

#else
    //test open close irq
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)gpio_input_tester[i].pin, HAL_GPIO_DIR_IN, 1);
    gpio_set_irq((enum HAL_GPIO_PIN_T)gpio_input_tester[0].pin,HAL_GPIO_IRQ_POLARITY_HIGH_RISING,true);
    test_pin_out_level(PIN_OUT_1, true);
    osDelay(1);
    test_pin_out_level(PIN_OUT_1, false);
    gpio_set_irq((enum HAL_GPIO_PIN_T)gpio_input_tester[0].pin,HAL_GPIO_IRQ_POLARITY_HIGH_RISING,false);//close irq
    osDelay(1);
    gpio_set_irq((enum HAL_GPIO_PIN_T)gpio_input_tester[0].pin,HAL_GPIO_IRQ_POLARITY_LOW_FALLING,true);//open irq
    return;
#endif

}
#endif
