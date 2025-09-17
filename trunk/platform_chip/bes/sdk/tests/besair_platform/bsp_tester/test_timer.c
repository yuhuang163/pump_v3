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
#ifdef TEST_TIMER
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#define IGNORE_HAL_TIMER_RAW_API_CHECK
#include "hal_timer_raw.h"
#include "hal_timer_fast_irq.h"
#include "hal_timer_user_irq.h"

#define USERTIMER_MS_TO_TICKS(ms)           ((ms) * ((uint32_t)6000000 / 1000))


#define ONESLOT_MS_DEFAULT                  (2000)
#define TEST_ONESLOT_DELAY_DEFAULT          (ONESLOT_MS_DEFAULT + 500)

#define PERIODIC_MS_DEFAULT                 (1000)
#define TEST_PERIODIC_DELAY_DEFAULT         (PERIODIC_MS_DEFAULT * 3)

#define PIN_OUT_1               (HAL_GPIO_PIN_P2_4)
#define PIN_OUT_2               (HAL_GPIO_PIN_P2_5)

static const struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_output_tester[] = {
    {PIN_OUT_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    {PIN_OUT_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
};

static void test_pin_out_level(enum HAL_GPIO_PIN_T pin, bool hi)
{
    if(hi) {
        hal_gpio_pin_set(pin);
    } else {
        hal_gpio_pin_clr(pin);
    }
}


static uint32_t slow_timer_old_time = 0;
static uint32_t slow_timer_new_time = 0;
static uint32_t user_timer_old_time = 0;
static uint32_t user_timer_new_time = 0;


static osThreadId_t test_timer_thread_id = NULL;

static void slow_timer_irq_handler(uint32_t elapsed)
{
    TRACE(2,"%s , 0x%x", __FUNCTION__, elapsed);
    osSignalSet(test_timer_thread_id, 1);
}

static void acon_timer_irq_handler(uint32_t elapsed)
{
    TRACE(2,"%s , 0x%x", __FUNCTION__, elapsed);
    osSignalSet(test_timer_thread_id, 1);
}

static void slow_timer_periodic_irq_handler(uint32_t elapsed)
{
    static bool hi = 0;
    test_pin_out_level(PIN_OUT_1, (hi = !hi));

    slow_timer_new_time = hal_fast_sys_timer_get();
    if(slow_timer_old_time) {
        TRACE(2, "slow_timer periodic run time: %dms", FAST_TICKS_TO_MS(slow_timer_new_time - slow_timer_old_time));
    }
    TRACE(2,"%s , 0x%x", __FUNCTION__, elapsed);
    slow_timer_old_time = slow_timer_new_time;
}

static void acon_timer_periodic_irq_handler(uint32_t elapsed)
{
    static bool hi2 = 0;
    test_pin_out_level(PIN_OUT_2, (hi2 = !hi2));

    user_timer_new_time = hal_fast_sys_timer_get();
    if(user_timer_old_time) {
        TRACE(2, "acon_timer periodic run time: %dms", FAST_TICKS_TO_MS(user_timer_new_time - user_timer_old_time));
    }
    TRACE(2,"%s , 0x%x", __FUNCTION__, elapsed);
    user_timer_old_time = user_timer_new_time;
}

//timer01 作为时间戳；
//fast timer:only freerunning
//aon timer:系统 sleep 
void test_timer(void)
{
   
    uint32_t stime;
    uint32_t etime;
    osEvent evt;
    test_timer_thread_id = osThreadGetId();

    hal_iomux_init(gpio_output_tester, 2);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)gpio_output_tester[0].pin, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)gpio_output_tester[1].pin, HAL_GPIO_DIR_OUT, 0);



    /* oneshot mode */
    TRACE(3,"%s timer oneshot[%dms] start.", __FUNCTION__, ONESLOT_MS_DEFAULT);    
    hal_timer_setup(HAL_TIMER_TYPE_ONESHOT, slow_timer_irq_handler);
    stime = hal_fast_sys_timer_get();
    hal_timer_start(MS_TO_TICKS(ONESLOT_MS_DEFAULT));



    if(test_timer_thread_id != NULL) {
        evt = osSignalWait(0x0, 5000);
        etime = hal_fast_sys_timer_get();
// get role from signal value
        if (evt.status == osEventSignal){
            TRACE(2, "check fast_timer freerun time: %dms", FAST_TICKS_TO_MS(etime - stime));
         }else if (evt.status == osEventTimeout){
            TRACE(2, "osEventTimeout");
        }
        TRACE(2, "slow_timer run time: %dms", FAST_TICKS_TO_MS(etime - stime));
        TRACE(1, "\n");
    }
    hal_user_timer_setup(HAL_TIMER_TYPE_ONESHOT, acon_timer_irq_handler);
    stime = hal_fast_sys_timer_get();
    hal_user_timer_start(USERTIMER_MS_TO_TICKS(ONESLOT_MS_DEFAULT));

    if(test_timer_thread_id != NULL) {
        evt = osSignalWait(0x0, 6000);
        etime = hal_fast_sys_timer_get();
        if (evt.status == osEventSignal){
            TRACE(2, "check fast_timer freerun time: %dms", FAST_TICKS_TO_MS(etime - stime));
        }else if (evt.status == osEventTimeout){
            TRACE(2, "osEventTimeout");
        }
        TRACE(2, "acon_timer run time: %dms", FAST_TICKS_TO_MS(etime - stime));

        TRACE(3,"%s timer oneshot end.", __FUNCTION__);
        TRACE(1, "\n");
    }
    /* periodic mode */
    TRACE(3,"%s timer periodic[%dms] start.", __FUNCTION__, PERIODIC_MS_DEFAULT);
    hal_timer_setup(HAL_TIMER_TYPE_PERIODIC, slow_timer_periodic_irq_handler);
    hal_timer_start(MS_TO_TICKS(PERIODIC_MS_DEFAULT));
    //osDelay(TEST_PERIODIC_DELAY_DEFAULT);
    //hal_timer_stop();
    //TRACE(1, "\n");

    slow_timer_old_time = slow_timer_new_time = 0;
    user_timer_old_time = user_timer_new_time = 0;
    hal_user_timer_setup(HAL_TIMER_TYPE_PERIODIC, acon_timer_periodic_irq_handler);
    hal_user_timer_start(USERTIMER_MS_TO_TICKS(PERIODIC_MS_DEFAULT));
    //osDelay(TEST_PERIODIC_DELAY_DEFAULT);
    //hal_user_timer_stop();
    //TRACE(3,"%s timer periodic end.", __FUNCTION__);
    //TRACE(1, "\n");


}
#endif


