/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#include <stdio.h>
#include <stdarg.h>
#include "plat_types.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "eshell.h"
#include "app_eshell.h"
#include "pmu.h"


static void do_gpio_set(int argc, char *argv[])
{
    uint32_t gpionum = 0, gpiopin = 0, value = 0;
    struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_gpio;

    if (argc < 3) {
        eshell_putstring("Usage: gpioset gpio_num <0|1>\r\n");
        return;
    }

    gpionum = atoi(argv[1]);
    if (gpionum % 10 > 7) {
        eshell_putstring("GPIO number must be x0 ~ x7.\r\n");
        return;
    }

    gpiopin = gpionum / 10 * 8 + gpionum % 10;
    if (gpiopin > HAL_GPIO_PIN_NUM) {
        eshell_putstring("GPIO %d is invalid!\r\n", gpionum);
        return;
    }

    value = atoi(argv[2]);
    if ((value != 0) && (value != 1)) {
        eshell_putstring("GPIO value must be 0 or 1.\r\n", value);
        return;
    }

    cfg_hw_gpio.pin = gpiopin;
    cfg_hw_gpio.function = HAL_IOMUX_FUNC_AS_GPIO;
    cfg_hw_gpio.volt = HAL_IOMUX_PIN_VOLTAGE_VIO;
    cfg_hw_gpio.pull_sel = HAL_IOMUX_PIN_NOPULL;
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_hw_gpio, 1);

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_hw_gpio.pin, HAL_GPIO_DIR_OUT, value);
    eshell_putstring("GPIO %d set to %d.\r\n", gpionum, value);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "gpioset", "gpioset gpio_num <0|1>", do_gpio_set);

static void do_gpio_get(int argc, char *argv[])
{
    uint32_t gpionum = 0, gpiopin = 0, value = 0;
    struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_gpio;

    if (argc < 2) {
        eshell_putstring("Usage: gpioget gpio_num\r\n");
        return;
    }

    gpionum = atoi(argv[1]);
    if (gpionum % 10 > 7) {
        eshell_putstring("GPIO number must be x0 ~ x7.\r\n");
        return;
    }

    gpiopin = gpionum / 10 * 8 + gpionum % 10;
    if (gpiopin > HAL_GPIO_PIN_NUM) {
        eshell_putstring("GPIO %d is invalid!\r\n", gpionum);
        return;
    }

    cfg_hw_gpio.pin = gpiopin;
    cfg_hw_gpio.function = HAL_IOMUX_FUNC_AS_GPIO;
    cfg_hw_gpio.volt = HAL_IOMUX_PIN_VOLTAGE_VIO;
    cfg_hw_gpio.pull_sel = HAL_IOMUX_PIN_NOPULL;
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_hw_gpio, 1);

    value = hal_gpio_pin_get_val(gpiopin);
    eshell_putstring("GPIO %d value is %d.\r\n", gpionum, value);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "gpioget", "gpioget gpio_num", do_gpio_get);

#ifdef CHIP_BEST1600

static void do_pmu_gpio_set(int argc, char *argv[])
{
    uint32_t gpionum = 0, gpiopin = 0, value = 0;
    struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_gpio;

    if (argc < 3) {
        eshell_putstring("Usage: pmugpioset gpio_num <0|1>\r\n");
        return;
    }

    gpionum = atoi(argv[1]);
    if (gpionum % 10 > 7) {
        eshell_putstring("PMU GPIO number must be x0 ~ x7.\r\n");
        return;
    }

    gpiopin = HAL_GPIO_PIN_PMU_P0_0 + gpionum / 10 * 8 + gpionum % 10;
    if (gpiopin >= HAL_GPIO_PIN_PMU_NUM) {
        eshell_putstring("PMU GPIO %d is invalid!\r\n", gpionum);
        return;
    }

    value = atoi(argv[2]);
    if ((value != 0) && (value != 1)) {
        eshell_putstring("PMU GPIO value must be 0 or 1.\r\n", value);
        return;
    }

    cfg_hw_gpio.pin = gpiopin;
    cfg_hw_gpio.function = HAL_IOMUX_FUNC_AS_GPIO;
    cfg_hw_gpio.volt = HAL_IOMUX_PIN_VOLTAGE_VIO;
    cfg_hw_gpio.pull_sel = HAL_IOMUX_PIN_NOPULL;
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_hw_gpio, 1);

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_hw_gpio.pin, HAL_GPIO_DIR_OUT, value);
    eshell_putstring("PMU GPIO %d set to %d.\r\n", gpionum, value);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "pmugpioset", "pmugpioset gpio_num <0|1>", do_pmu_gpio_set);

static void do_pmu_gpio_get(int argc, char *argv[])
{
    uint32_t gpionum = 0, gpiopin = 0, value = 0;
    struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_gpio;

    if (argc < 2) {
        eshell_putstring("Usage: pmugpioget gpio_num\r\n");
        return;
    }

    gpionum = atoi(argv[1]);
    if (gpionum % 10 > 7) {
        eshell_putstring("PMU GPIO number must be x0 ~ x7.\r\n");
        return;
    }

    gpiopin = HAL_GPIO_PIN_PMU_P0_0 + gpionum / 10 * 8 + gpionum % 10;
    if (gpiopin > HAL_GPIO_PIN_PMU_NUM) {
        eshell_putstring("PMU GPIO %d is invalid!\r\n", gpionum);
        return;
    }

    cfg_hw_gpio.pin = gpiopin;
    cfg_hw_gpio.function = HAL_IOMUX_FUNC_AS_GPIO;
    cfg_hw_gpio.volt = HAL_IOMUX_PIN_VOLTAGE_VIO;
    cfg_hw_gpio.pull_sel = HAL_IOMUX_PIN_NOPULL;
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_hw_gpio, 1);

    value = hal_gpio_pin_get_val(gpiopin);
    eshell_putstring("PMU GPIO %d value is %d.\r\n", gpionum, value);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "pmugpioget", "pmugpioget gpio_num", do_pmu_gpio_get);

#endif //CHIP_BEST1600

#endif //UTILS_ESHELL_EN
