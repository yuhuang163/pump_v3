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
#ifndef GPIO_TESTER_EXT_H
#define GPIO_TESTER_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio_tester.h"

#define gpio_CMD_HELP_INFO                                                 "\n\
        help - an interface to the gpio test command manual                 \n\
SYNOPSIS                                                                    \n\
        gpio,help                                                           \n\
        gpio,[command],[pin number]                                         \n\
                command options                                             \n\
                        output_low, output_high, input, toggle              \n\
                        irq_low, irq_high, irq_fall, irq_rise, irq_disable  \n\
                pin number options                                          \n\
                        00 ~ 07, 10 ~ 17, 20 ~ 27, 30 ~ 37, 40 ~ 47,        \n\
                        50 ~ 57, 60 ~ 67, 70 ~ 77, 80 ~ 87, 90 ~ 97         \n\
EXAMPLES                                                                    \n\
        gpio,help                                                           \n\
                get help manual of gpio test command                        \n\
        gpio,output_high,15                                                 \n\
                set pin 1_5(p15) as output high (internal pull-up)          \n\
        gpio,output_low,23                                                  \n\
                set pin 2_3(p23) as output low  (internal pull-down)        \n\
        gpio,input,67                                                       \n\
                set pin 6_7(p67) as input (Hi-Z) and read its value         \n\
        gpio,toggle,10                                                      \n\
                toggle pin 1_0(p10) every specific time period              \n\
        gpio,irq_low,11                                                     \n\
                set pin 1_1(p11) as interrupt port (low level triggered)    \n\
        gpio,irq_high,11                                                    \n\
                set pin 1_1(p11) as interrupt port (high level triggered)   \n\
        gpio,irq_fall,11                                                    \n\
                set pin 1_1(p11) as interrupt port (falling edge triggered) \n\
        gpio,irq_rise,11                                                    \n\
                set pin 1_1(p11) as interrupt port (rising edge triggered)  \n\
        gpio,irq_disable,11                                                 \n\
                disable pin 1_1(p11) interrupt port                       \n\n\
"

/**
 * \brief gpio test function
 * \param [in]  params  parameters received as strings from uart port
 */
void gpio_tester(const char **params);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_TESTER_EXT_H */