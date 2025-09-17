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
#ifdef GPIO_TEST

#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_gpio.h"

#define GPIO_P0_TEST
#define GPIO_P1_TEST
#define GPIO_P2_TEST
#define GPIO_P3_TEST
//#define GPIO_P4_TEST
//#define GPIO_P5_TEST
//#define GPIO_P6_TEST
//#define GPIO_P7_TEST
//#define GPIO_P8_TEST
//#define GPIO_P9_TEST

#define  GPIO_TEST_IRQ
//#define  GPIO_BOTH_EDGE_IRQ_TEST

#ifdef   GPIO_TEST_IRQ
#define  GPIO_IRQ_TEST_PIN_IN           HAL_IOMUX_PIN_P1_6
#define  GPIO_IRQ_TEST_PIN_OUT          HAL_IOMUX_PIN_P2_7
#define  GPIO_IRQ_TYPE                  HAL_GPIO_IRQ_POLARITY_LOW_FALLING
#define  _MARCO_TO_STR(marco_level2)    #marco_level2
#define  MARCO_TO_STR(marco_level1)     _MARCO_TO_STR(marco_level1)
#endif

static void gpio_test_output(void)
{
    TRACE_IMM(0, "  ");
    TRACE_IMM(0, "------------------------gpio test started------------------------");
    TRACE_IMM(0, "***log will stop output, please use an oscilloscope to view the waveform***");
    TRACE_IMM(0, "  ");

    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
#ifdef GPIO_P0_TEST
        {HAL_IOMUX_PIN_P0_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#ifdef GPIO_P1_TEST
        {HAL_IOMUX_PIN_P1_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#ifdef GPIO_P2_TEST
        {HAL_IOMUX_PIN_P2_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#ifdef GPIO_P3_TEST
        {HAL_IOMUX_PIN_P3_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P3_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P3_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P3_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P3_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P3_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#if !defined(CHIP_BEST2002) && !defined(CHIP_BEST1501)
        {HAL_IOMUX_PIN_P3_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P3_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#endif
#ifdef GPIO_P4_TEST
        {HAL_IOMUX_PIN_P4_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P4_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P4_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P4_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P4_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P4_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P4_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P4_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#ifdef GPIO_P5_TEST
        {HAL_IOMUX_PIN_P5_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P5_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P5_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P5_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P5_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P5_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P5_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P5_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#ifdef GPIO_P6_TEST
        {HAL_IOMUX_PIN_P6_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P6_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P6_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P6_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P6_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P6_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P6_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P6_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#ifdef GPIO_P7_TEST
        {HAL_IOMUX_PIN_P7_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P7_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P7_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P7_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P7_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P7_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P7_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P7_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#ifdef GPIO_P8_TEST
        {HAL_IOMUX_PIN_P8_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P8_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P8_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P8_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P8_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P8_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P8_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P8_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
#ifdef GPIO_P9_TEST
        {HAL_IOMUX_PIN_P9_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P9_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P9_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P9_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P9_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P9_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P9_6, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P9_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
#endif
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));

#ifdef GPIO_P0_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_7, HAL_GPIO_DIR_OUT, 1);
#endif
#ifdef GPIO_P1_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P1_7, HAL_GPIO_DIR_OUT, 1);
#endif
#ifdef GPIO_P2_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P2_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P2_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P2_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P2_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P2_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P2_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P2_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P2_7, HAL_GPIO_DIR_OUT, 1);
#endif
#ifdef GPIO_P3_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_5, HAL_GPIO_DIR_OUT, 1);
#if !defined(CHIP_BEST2002) && !defined(CHIP_BEST1501)
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_7, HAL_GPIO_DIR_OUT, 1);
#endif
#endif
#ifdef GPIO_P4_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P4_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P4_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P4_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P4_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P4_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P4_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P4_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P4_7, HAL_GPIO_DIR_OUT, 1);
#endif
#ifdef GPIO_P5_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P5_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P5_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P5_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P5_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P5_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P5_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P5_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P5_7, HAL_GPIO_DIR_OUT, 1);
#endif
#ifdef GPIO_P6_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P6_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P6_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P6_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P6_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P6_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P6_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P6_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P6_7, HAL_GPIO_DIR_OUT, 1);
#endif
#ifdef GPIO_P7_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P7_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P7_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P7_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P7_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P7_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P7_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P7_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P7_7, HAL_GPIO_DIR_OUT, 1);
#endif
#ifdef GPIO_P8_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P8_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P8_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P8_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P8_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P8_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P8_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P8_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P8_7, HAL_GPIO_DIR_OUT, 1);
#endif
#ifdef GPIO_P9_TEST
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P9_0, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P9_1, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P9_2, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P9_3, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P9_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P9_5, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P9_6, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P9_7, HAL_GPIO_DIR_OUT, 1);
#endif
    while (1) {
#ifdef GPIO_P0_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P0_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P0_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P0_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P0_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P0_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P0_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P0_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P0_7);
#endif
#ifdef GPIO_P1_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P1_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P1_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P1_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P1_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P1_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P1_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P1_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P1_7);
#endif
#ifdef GPIO_P2_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P2_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P2_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P2_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P2_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P2_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P2_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P2_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P2_7);
#endif
#ifdef GPIO_P3_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P3_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P3_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P3_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P3_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P3_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P3_5);
#if !defined(CHIP_BEST2002) && !defined(CHIP_BEST1501)
        hal_gpio_pin_set(HAL_GPIO_PIN_P3_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P3_7);
#endif
#endif
#ifdef GPIO_P4_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P4_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P4_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P4_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P4_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P4_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P4_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P4_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P4_7);
#endif
#ifdef GPIO_P5_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P5_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P5_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P5_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P5_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P5_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P5_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P5_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P5_7);
#endif
#ifdef GPIO_P6_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P6_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P6_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P6_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P6_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P6_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P6_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P6_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P6_7);
#endif
#ifdef GPIO_P7_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P7_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P7_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P7_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P7_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P7_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P7_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P7_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P7_7);
#endif
#ifdef GPIO_P8_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P8_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P8_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P8_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P8_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P8_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P8_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P8_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P8_7);
#endif
#ifdef GPIO_P9_TEST
        hal_gpio_pin_set(HAL_GPIO_PIN_P9_0);
        hal_gpio_pin_set(HAL_GPIO_PIN_P9_1);
        hal_gpio_pin_set(HAL_GPIO_PIN_P9_2);
        hal_gpio_pin_set(HAL_GPIO_PIN_P9_3);
        hal_gpio_pin_set(HAL_GPIO_PIN_P9_4);
        hal_gpio_pin_set(HAL_GPIO_PIN_P9_5);
        hal_gpio_pin_set(HAL_GPIO_PIN_P9_6);
        hal_gpio_pin_set(HAL_GPIO_PIN_P9_7);
#endif
        hal_sys_timer_delay(MS_TO_TICKS(50));

#ifdef GPIO_P0_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P0_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P0_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P0_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P0_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P0_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P0_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P0_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P0_7);
#endif
#ifdef GPIO_P1_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P1_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P1_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P1_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P1_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P1_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P1_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P1_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P1_7);
#endif
#ifdef GPIO_P2_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P2_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P2_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P2_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P2_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P2_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P2_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P2_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P2_7);
#endif
#ifdef GPIO_P3_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P3_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P3_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P3_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P3_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P3_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P3_5);
#if !defined(CHIP_BEST2002) && !defined(CHIP_BEST1501)
        hal_gpio_pin_clr(HAL_GPIO_PIN_P3_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P3_7);
#endif
#endif
#ifdef GPIO_P4_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P4_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P4_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P4_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P4_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P4_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P4_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P4_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P4_7);
#endif
#ifdef GPIO_P5_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P5_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P5_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P5_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P5_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P5_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P5_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P5_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P5_7);
#endif
#ifdef GPIO_P6_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P6_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P6_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P6_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P6_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P6_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P6_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P6_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P6_7);
#endif
#ifdef GPIO_P7_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P7_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P7_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P7_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P7_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P7_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P7_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P7_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P7_7);
#endif
#ifdef GPIO_P8_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P8_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P8_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P8_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P8_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P8_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P8_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P8_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P8_7);
#endif
#ifdef GPIO_P9_TEST
        hal_gpio_pin_clr(HAL_GPIO_PIN_P9_0);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P9_1);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P9_2);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P9_3);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P9_4);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P9_5);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P9_6);
        hal_gpio_pin_clr(HAL_GPIO_PIN_P9_7);
#endif
        hal_sys_timer_delay(MS_TO_TICKS(50));
    }
}



#ifdef GPIO_TEST_IRQ
int gpio_irq_count = 0;
static void gpio_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    TRACE(0, "%s: pin[%d] irq triggered", __func__, pin);
    gpio_irq_count ++ ;
}

static void gpio_test_irq()
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map[] = {
            {GPIO_IRQ_TEST_PIN_IN,  HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
            {GPIO_IRQ_TEST_PIN_OUT, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE}};
    hal_iomux_init(gpio_map, ARRAY_SIZE(gpio_map));
    hal_gpio_pin_set_dir(GPIO_IRQ_TEST_PIN_IN,  HAL_GPIO_DIR_IN,  1);
    hal_gpio_pin_set_dir(GPIO_IRQ_TEST_PIN_OUT, HAL_GPIO_DIR_OUT, 1);
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg =
#if (GPIO_IRQ_TYPE == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) || (GPIO_IRQ_TYPE == HAL_GPIO_IRQ_POLARITY_HIGH_RISING)
        {1, 0, HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, GPIO_IRQ_TYPE, gpio_irq_callback };
#else
        {1, 0, HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE, GPIO_IRQ_TYPE, gpio_irq_callback };
#endif
    hal_gpio_setup_irq(GPIO_IRQ_TEST_PIN_IN, &gpio_irq_cfg);
    TRACE(0, "Start test gpio irq ... ");
    TRACE(0, "Irq pin is %s, irq type is %s ", MARCO_TO_STR(GPIO_IRQ_TEST_PIN_IN), MARCO_TO_STR(GPIO_IRQ_TYPE));
    TRACE(0, "Please connect %s to %s", MARCO_TO_STR(GPIO_IRQ_TEST_PIN_OUT),MARCO_TO_STR(GPIO_IRQ_TEST_PIN_IN));
#define GPIO_IRQ_TEST_COUNT  60
    int count = GPIO_IRQ_TEST_COUNT;
    while(count--){
        hal_sys_timer_delay(MS_TO_TICKS(10));
        hal_gpio_pin_clr(GPIO_IRQ_TEST_PIN_OUT);
        hal_sys_timer_delay(MS_TO_TICKS(10));
        hal_gpio_pin_set(GPIO_IRQ_TEST_PIN_OUT);
    }
    gpio_irq_cfg.irq_enable = 0;
    hal_gpio_setup_irq(GPIO_IRQ_TEST_PIN_IN, &gpio_irq_cfg);
    TRACE(0,"gpio_irq_count = %d",gpio_irq_count);
    if (gpio_irq_count == GPIO_IRQ_TEST_COUNT){
        TRACE(0,"gpio irq test PASS");
    } else {
        TRACE(0,"gpio irq test FAIL");
    }
}

#ifdef GPIO_BOTH_EDGE_IRQ_TEST
static int both_edge_irq_cnt = 0;
static int both_edge_irq_f_cnt = 0;

static void gpio_both_edge_callback(enum HAL_GPIO_PIN_T pin)
{
    TRACE(0, "[%s]: pin[%d] irq triggered", __func__, pin);

    if (hal_gpio_both_edge_irq_type_get(pin) == HAL_GPIO_BOTH_EDGE_IRQ_TYPE_RISING) {
        both_edge_irq_cnt++;
    } else if (hal_gpio_both_edge_irq_type_get(pin) == HAL_GPIO_BOTH_EDGE_IRQ_TYPE_FALLING) {
        both_edge_irq_f_cnt++;
    }
}

static void gpio_test_irq_both_edge(void)
{
#define GPIO_BOTH_EDGE_IRQ_TEST_PIN_IN              HAL_IOMUX_PIN_P0_0
#define GPIO_BOTH_EDGE_IRQ_TEST_PIN_OUT             HAL_IOMUX_PIN_P0_1

    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg;
    const struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map[] = {
        {GPIO_BOTH_EDGE_IRQ_TEST_PIN_IN,  HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {GPIO_BOTH_EDGE_IRQ_TEST_PIN_OUT, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL}
    };

    hal_iomux_init(gpio_map, ARRAY_SIZE(gpio_map));
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)GPIO_BOTH_EDGE_IRQ_TEST_PIN_IN, HAL_GPIO_DIR_IN, 0);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)GPIO_BOTH_EDGE_IRQ_TEST_PIN_OUT, HAL_GPIO_DIR_OUT, 1);

    gpio_irq_cfg.irq_debounce = false;
    gpio_irq_cfg.irq_enable = true;
    gpio_irq_cfg.irq_handler = gpio_both_edge_callback;
    gpio_irq_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_EDGE_BOTH;
    gpio_irq_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;

    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)GPIO_BOTH_EDGE_IRQ_TEST_PIN_IN, &gpio_irq_cfg);

    TRACE(0, "[%s]: Start Irq pin is %s", __func__, MARCO_TO_STR(GPIO_BOTH_EDGE_IRQ_TEST_PIN_IN));
    TRACE(0, "[%s]: Please connect %s to %s", __func__, MARCO_TO_STR(GPIO_BOTH_EDGE_IRQ_TEST_PIN_OUT),MARCO_TO_STR(GPIO_BOTH_EDGE_IRQ_TEST_PIN_IN));

#define GPIO_BOTH_EDGE_IRQ_TEST_CNT   50
    int count = GPIO_BOTH_EDGE_IRQ_TEST_CNT;

    while (count--) {
        hal_sys_timer_delay(MS_TO_TICKS(10));
        hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)GPIO_BOTH_EDGE_IRQ_TEST_PIN_OUT);
        hal_sys_timer_delay(MS_TO_TICKS(10));
        hal_gpio_pin_set((enum HAL_GPIO_PIN_T)GPIO_BOTH_EDGE_IRQ_TEST_PIN_OUT);
    }

    gpio_irq_cfg.irq_enable = false;
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)GPIO_BOTH_EDGE_IRQ_TEST_PIN_IN, &gpio_irq_cfg);

    if (both_edge_irq_cnt == GPIO_BOTH_EDGE_IRQ_TEST_CNT && both_edge_irq_f_cnt == GPIO_BOTH_EDGE_IRQ_TEST_CNT) {
        TRACE(0, "[%s]: PASS", __func__);
    } else {
        TRACE(0, "[%s]: FAIL edge_irq_cnt:%d edge_f_irq_cnt:%d", __func__, both_edge_irq_cnt, both_edge_irq_f_cnt);
    }
}
#endif
#endif

void gpio_test(void)
{
#ifdef GPIO_TEST_IRQ
    gpio_test_irq();
#ifdef GPIO_BOTH_EDGE_IRQ_TEST
    gpio_test_irq_both_edge();
#endif
#endif
    gpio_test_output();
}

#endif

