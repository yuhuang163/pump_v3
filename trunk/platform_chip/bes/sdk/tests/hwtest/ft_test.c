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
#include "hal_dma.h"
#include "hal_cmu.h"
#include "string.h"
#ifndef NOSTD
#include "math.h"
#endif
#include "hal_timer.h"
#include "hal_gpio.h"
#include "hal_uart.h"

#define GPIO_INPUT      HAL_IOMUX_PIN_P0_0
#define GPIO_OUTPUT     HAL_IOMUX_PIN_P0_1

const struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_test_gpio[2] = {
    {GPIO_INPUT, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
    {GPIO_OUTPUT, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
};

static volatile bool ready_flag = false;
static volatile bool start_timer = false;
uint32_t check_ready_flag_timer_start = 0;

static uint8_t uart_rx_buf[512]; 

static void gpio_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    start_timer = false;
    ready_flag = true;
    TRACE(0, "IRQ CHECKED");
}

static unsigned int app_uart_rx_process(unsigned char *buf, unsigned int len)
{
    if (buf == NULL) {
        return 1;
    }

    buf[len] = 0; 

    if(strncmp((char*)buf, "START", 5) == 0){
        TRACE(0, "recv START");
        hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)GPIO_OUTPUT);
    }else if(strncmp((char*)buf, "END", 5) == 0){
        TRACE(0, "recv END");
         hal_gpio_pin_set((enum HAL_GPIO_PIN_T)GPIO_OUTPUT);
    }
    return 0;
}

void ft_test_main(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_cfg;
    hal_iomux_init(cfg_test_gpio, ARRAY_SIZE(cfg_test_gpio));
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)GPIO_INPUT, HAL_GPIO_DIR_IN, 0);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)GPIO_OUTPUT, HAL_GPIO_DIR_OUT, 1);

    gpio_cfg.irq_enable = true;
    gpio_cfg.irq_debounce = true;
    gpio_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpio_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpio_cfg.irq_handler = gpio_irq_handler; 
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)GPIO_INPUT, &gpio_cfg);

    hal_trace_rx_open(uart_rx_buf, sizeof(uart_rx_buf), app_uart_rx_process);
    //hal_sys_timer_open();
    TRACE(1, "enter FT_test \n");
    while(1){
        if(ready_flag){
            if(!start_timer){
                check_ready_flag_timer_start = hal_sys_timer_get();
                start_timer = true;
                TRACE(TR_ATTR_NO_TS, "start:%d", check_ready_flag_timer_start);
            }

            if(start_timer){
                if((TICKS_TO_MS(hal_sys_timer_get() - check_ready_flag_timer_start)) > 80){          
                    if(hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)GPIO_INPUT)){
                        TRACE(TR_ATTR_NO_TS, "READY\n");
                    }else{
                         TRACE(TR_ATTR_NO_TS, "INVALID IRQ \n");
                    }
                    ready_flag = false;
                }
            }
        }
    }
}

