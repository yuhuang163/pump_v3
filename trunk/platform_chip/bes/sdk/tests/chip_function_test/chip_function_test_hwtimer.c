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
#ifdef CHIP_FUNCTION_TEST_HWTIMER
#include "string.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_timer_fast_irq.h"
#include "hal_timer_user_irq.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "pmu.h"
#include "hal_gpio.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_sysfreq.h"

#define HWTIMER_TEST_FUNCTION

#ifndef TEST_TIMER_IND
#define TEST_TIMER_IND     1
#endif

uint32_t _ms_to_ticks(bool is_fast, uint32_t ms)
{
    if(is_fast)
    {
        return MS_TO_FAST_TICKS(ms);
    }
    else
    {
        return MS_TO_TICKS(ms);
    }
}

uint32_t _ticks_to_ms(bool is_fast, uint32_t ticks)
{
    if(is_fast)
    {
        return FAST_TICKS_TO_MS(ticks);
    }
    else
    {
        return TICKS_TO_MS(ticks);
    }
}

uint32_t _us_to_ticks(bool is_fast, uint32_t us)
{
    if(is_fast)
    {
        return US_TO_FAST_TICKS(us);
    }
    else
    {
        return US_TO_TICKS(us);
    }
}

uint32_t _ticks_to_us(bool is_fast, uint32_t ticks)
{
    if(is_fast)
    {
        return FAST_TICKS_TO_US(ticks);
    }
    else
    {
        return TICKS_TO_US(ticks);
    }
}

#ifdef HWTIMER_TEST_FUNCTION

#define TIMER_INTERVAL_MS   500
#define TIMER_DELAY_MS      8

// maximum allowable deviation
#define TIMER_ALLOWABLE_DEVIATION_MAX   5
#define LOOP_NR             2
#define TEST_STEP_MAX       4
#define TIME_WATI_TIMEOUT  (TIMER_INTERVAL_MS*LOOP_NR)*TEST_STEP_MAX
//-------------------------------------------------------------------
void hal_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);

void hal_timer_start(uint32_t load);

void hal_timer_stop(void);

void hal_timer_continue(void);

int hal_timer_is_enabled(void);

void hal_timer_reload(uint32_t load);

uint32_t hal_timer_get_raw_value(void);

int hal_timer_irq_active(void);

int hal_timer_irq_pending(void);

uint32_t hal_timer_get_elapsed_time(void);
//-------------------------------------------------------------------
#define TEIMER_TEST_RESULT_SETUP            (0x1<<0)
#define TEIMER_TEST_RESULT_START            (0x1<<1)
#define TEIMER_TEST_RESULT_STOP             (0x1<<2)
#define TEIMER_TEST_RESULT_CONTINUE         (0x1<<3)
#define TEIMER_TEST_RESULT_IS_ENABLE        (0x1<<4)
#define TEIMER_TEST_RESULT_RELOAD           (0x1<<5)
#define TEIMER_TEST_RESULT_GET_RAW_VAL      (0x1<<6)
#define TEIMER_TEST_RESULT_IRQ_ACTIVE       (0x1<<7)
#define TEIMER_TEST_RESULT_IRQ_PENDING      (0x1<<8)
#define TEIMER_TEST_RESULT_GET_ELAPSED_TIME (0x1<<9)

#define TIMER_TEST_INVALID_ID 0xff
enum TIMER_ID
{
    TIMER_TEST_TIMER0 = 0,
    TIMER_TEST_TIMER1,
    TIMER_TEST_TIMER2,
    TIMER_TEST_TIMER_COUNT,
};

typedef void (* timer_setup)(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);
typedef void (* timer_start)(uint32_t load);
typedef void (* timer_stop)(void);
typedef void (* timer_continue)(void);
typedef int (* timer_is_enabled)(void);
typedef void (* timer_reload)(uint32_t load);

typedef uint32_t (* timer_get_raw_value)(void);
typedef int (* timer_irq_active)(void);
typedef int (* timer_irq_pending)(void);
typedef uint32_t (* timer_get_elapsed_time)(void);


typedef struct {
    uint8_t timer_name[32];
    enum TIMER_ID id;
    bool is_fast;
    timer_setup setup_fp;
    timer_start start_fp;
    timer_stop stop_fp;
    timer_continue continue_fp;
    timer_is_enabled is_enable_fp;
    timer_reload reload_fp;
    timer_get_raw_value get_raw_value_fp;
    timer_irq_active irq_active_fp;
    timer_irq_pending irq_pending_fp;
    timer_get_elapsed_time get_elapsed_time_fp;
}HWTIMER_TEST;

typedef struct {
    enum TIMER_ID id;
    uint32_t case_mark;
    uint32_t fail_mark;
}HWTIMER_TEST_RESULT;

HWTIMER_TEST hwtimer_test[] = {
{"TIMER0", TIMER_TEST_TIMER0, false,
      hal_timer_setup,
      hal_timer_start,
      hal_timer_stop,
      hal_timer_continue,
      hal_timer_is_enabled,
      hal_timer_reload,
      hal_timer_get_raw_value,
      hal_timer_irq_active,
      hal_timer_irq_pending,
      hal_timer_get_elapsed_time
      },
{"TIMER1", TIMER_TEST_TIMER1, true,
      hal_fast_timer_setup,
      hal_fast_timer_start,
      hal_fast_timer_stop,
      hal_fast_timer_continue,
      hal_fast_timer_is_enabled,
      hal_fast_timer_reload,
      hal_fast_timer_get_raw_value,
      hal_fast_timer_irq_active,
      hal_fast_timer_irq_pending,
      hal_fast_timer_get_elapsed_time
      },
{"TIMER2", TIMER_TEST_TIMER2, true,
      hal_user_timer_setup,
      hal_user_timer_start,
      hal_user_timer_stop,
      hal_user_timer_continue,
      hal_user_timer_is_enabled,
      hal_user_timer_reload,
      hal_user_timer_get_raw_value,
      hal_user_timer_irq_active,
      hal_user_timer_irq_pending,
      hal_user_timer_get_elapsed_time
      },
};
HWTIMER_TEST_RESULT test_result[TIMER_TEST_TIMER_COUNT] = {
{TIMER_TEST_TIMER0, 0x0, 0x0},
{TIMER_TEST_TIMER1, 0x0, 0x0},
{TIMER_TEST_TIMER2, 0x0, 0x0},
};
static uint32_t _start_timer = 0;
static uint8_t hwtimer_step = 0;
static uint32_t test_ind;
static uint32_t onshot_index;
static uint32_t periodic_index;
static uint32_t _get_ind(enum TIMER_ID id)
{
    uint32_t i;
    for(i = 0; i < sizeof(hwtimer_test)/sizeof(hwtimer_test[0]); i++)
    {
        if(id == hwtimer_test[i].id)
        {
            return i;
        }
    }
    return TIMER_TEST_INVALID_ID;
}


void _delay(bool is_fast, uint32_t delay_ms)
{

    uint32_t start_time;
    uint32_t end_time;

    if(is_fast)
    {
        start_time = hal_fast_sys_timer_get();
        do{
            end_time = hal_fast_sys_timer_get();
            if(FAST_TICKS_TO_MS(end_time - start_time) >= delay_ms)
            {
                break;
            }
        }while(1);
    }
    else
    {
        start_time = hal_sys_timer_get();
        do{
            end_time = hal_sys_timer_get();
            if(TICKS_TO_MS(end_time - start_time) >= delay_ms)
            {
                break;
            }
        }while(1);
    }

}

static void _oneshot_timer_handler(uint32_t elapsed)
{
    uint32_t ticks;
    uint32_t elapsed_ms;
    uint32_t dev_val;

    ticks = _ms_to_ticks(hwtimer_test[test_ind].is_fast, TIMER_INTERVAL_MS);
    elapsed_ms = _ticks_to_ms(hwtimer_test[test_ind].is_fast, elapsed);
    if(elapsed_ms != TIMER_INTERVAL_MS)
    {
        dev_val = TIMER_INTERVAL_MS > elapsed_ms ? \
                  TIMER_INTERVAL_MS - elapsed_ms : \
                  elapsed_ms - TIMER_INTERVAL_MS;

        if(dev_val > TIMER_ALLOWABLE_DEVIATION_MAX)
        {
            hwtimer_step = 5;
            TRACE(2,"%s: ERROR: deviation too large! elapsed: %d expected value: %d, index: %d",
                 __func__, elapsed_ms, TIMER_INTERVAL_MS, onshot_index);
            return;
        }
        else
        {
            TRACE(2,"%s: WARNING: elapsed: %d expected value: %d, index: %d",
                 __func__, elapsed_ms, TIMER_INTERVAL_MS, onshot_index);
        }
    }
    else
    {
        TRACE(2,"%s: elapsed_ms = %d, index = %d", __func__, elapsed_ms, onshot_index);
    }
    if(onshot_index < LOOP_NR)
    {
        hwtimer_test[test_ind].start_fp(ticks);
        if(hwtimer_step == 0)
        {
            hwtimer_step = 1;
            TRACE(2,"%s: hwtimer_step=%d. First timmer Intterupt.", __func__, hwtimer_step);
        }
    }
    else if(onshot_index < LOOP_NR*2)
    {
        hwtimer_test[test_ind].start_fp(ticks);
        if(hwtimer_step == 1)
        {
            hwtimer_step = 2;
            TRACE(2,"%s: hwtimer_step=%d", __func__, hwtimer_step);
        }
    }
    else
    {
        if(hwtimer_step == 2)
        {
            hwtimer_step = 3;
            TRACE(2,"%s: hwtimer_step=%d.Last timmer Intterupt.", __func__, hwtimer_step);
        }
        else
        {
            hwtimer_step = 4;
            TRACE(2,"%s: hwtimer_step=%d", __func__, hwtimer_step);
        }
    }
    onshot_index ++;
}

static void _periodic_timer_handler(uint32_t elapsed)
{
    uint32_t elapsed_ms;
    uint32_t dev_val = 0;

    elapsed_ms = _ticks_to_ms(hwtimer_test[test_ind].is_fast, elapsed);
    if(elapsed_ms != TIMER_INTERVAL_MS*periodic_index)
    {
        dev_val = TIMER_INTERVAL_MS*periodic_index > elapsed_ms ? \
                  TIMER_INTERVAL_MS*periodic_index - elapsed_ms : \
                  elapsed_ms - TIMER_INTERVAL_MS*periodic_index;

        if(dev_val > TIMER_ALLOWABLE_DEVIATION_MAX)
        {
            hwtimer_step = 5;
            hwtimer_test[test_ind].stop_fp();
            TRACE(2,"%s: ERROR: deviation too large! elapsed: %d expected value: %d, index: %d",
                 __func__, elapsed_ms, TIMER_INTERVAL_MS*periodic_index, periodic_index);
            return;
        }
        else
        {
            TRACE(2,"%s: WARNING: elapsed: %d expected value: %d, index: %d",
                 __func__, elapsed_ms, TIMER_INTERVAL_MS*periodic_index, periodic_index);
        }
    }
    else
    {
        TRACE(2,"%s: elapsed_ms = %d, index = %d", __func__, elapsed_ms, periodic_index);
    }

    if(periodic_index < LOOP_NR)
    {
        if(hwtimer_step == 0)
        {
            hwtimer_step = 1;
            TRACE(2,"%s: hwtimer_step=%d.First timmer Intterupt.", __func__, hwtimer_step);
        }
    }
    else if(periodic_index == LOOP_NR*2)
    {
        if(hwtimer_step == 1)
        {
            hwtimer_step = 2;
            hwtimer_test[test_ind].stop_fp();
            TRACE(2,"%s: hwtimer_step=%d, timer stop. Last timmer Intterupt.", __func__, hwtimer_step);
        }
    }
    else
    {
        if(hwtimer_step == 2)
        {
            hwtimer_step = 3;
            TRACE(2,"%s: periodic test can't stop! hwtimer_step=%d", __func__, hwtimer_step);
        }
    }
    periodic_index ++;
}

void _hwtimer_test_function(enum TIMER_ID id)
{
    uint32_t wait_start;
    uint32_t wait_end;
    uint32_t ticks;
    uint32_t elapsed_time;
    uint32_t delay_ticks;

    test_ind = _get_ind(id);
    onshot_index = 1;
    periodic_index = 1;

    ASSERT(id != TIMER_TEST_INVALID_ID, "%s: id(%d) is invlid", __func__, id);
    TRACE(1,"\n");
    TRACE(1, "%s TEST:", hwtimer_test[id].timer_name);
    TRACE(1,"\n");
    TRACE(1, "onshot test begin");

    // fail mark clear.
    test_result[test_ind].fail_mark = 0;

    // onshot test.
    hwtimer_step = 0;

    // Setup
    hwtimer_test[test_ind].setup_fp(HAL_TIMER_TYPE_ONESHOT,_oneshot_timer_handler);
    test_result[test_ind].case_mark |= TEIMER_TEST_RESULT_SETUP;

    // Start
    ticks = _ms_to_ticks(hwtimer_test[test_ind].is_fast, TIMER_INTERVAL_MS);
    hwtimer_test[test_ind].start_fp(ticks);
    test_result[test_ind].case_mark |= TEIMER_TEST_RESULT_START;

    //get_elapsed.
    _delay(hwtimer_test[test_ind].is_fast, TIMER_DELAY_MS);
    elapsed_time = hwtimer_test[test_ind].get_elapsed_time_fp();
    test_result[test_ind].case_mark |= TEIMER_TEST_RESULT_GET_ELAPSED_TIME;
    if(TIMER_DELAY_MS != _ticks_to_ms(hwtimer_test[test_ind].is_fast, elapsed_time))
    {
       TRACE(1, "test get_elapsed_time fail! elapsed_time = %d.", _ticks_to_ms(hwtimer_test[test_ind].is_fast, elapsed_time));
       test_result[test_ind].fail_mark |= TEIMER_TEST_RESULT_GET_ELAPSED_TIME;
    }
    else
    {
        TRACE(1, "test get_elapsed_time pass elapsed_time = %d.", _ticks_to_ms(hwtimer_test[test_ind].is_fast, elapsed_time));
    }

    //reload.
    delay_ticks = _ms_to_ticks(hwtimer_test[test_ind].is_fast, TIMER_INTERVAL_MS - TIMER_DELAY_MS);
    hwtimer_test[test_ind].reload_fp(delay_ticks);
    test_result[test_ind].case_mark |= TEIMER_TEST_RESULT_RELOAD;
    elapsed_time = hwtimer_test[test_ind].get_elapsed_time_fp();
    if(TIMER_DELAY_MS != _ticks_to_ms(hwtimer_test[test_ind].is_fast, elapsed_time))
    {
       TRACE(1, "test reload fail! elapsed_time = %d.", _ticks_to_ms(hwtimer_test[test_ind].is_fast, elapsed_time));
       test_result[test_ind].fail_mark |= TEIMER_TEST_RESULT_RELOAD;
    }
    else
    {
        TRACE(1, "test reload pass elapsed_time = %d.", _ticks_to_ms(hwtimer_test[test_ind].is_fast, elapsed_time));
    }

     wait_start = hal_fast_sys_timer_get();
    _start_timer = wait_start;
    while(1)
    {
        wait_end = hal_fast_sys_timer_get();
        if(FAST_TICKS_TO_MS(wait_end - wait_start) >= TIME_WATI_TIMEOUT &&
           hwtimer_step < 2)
        {
            TRACE(1, "oneshot test wait timeout!!! Need: %d(ms), Wait: %d, Step: %d",
                   (TIMER_INTERVAL_MS*LOOP_NR*2),
                   FAST_TICKS_TO_MS(wait_end - wait_start),
                   hwtimer_step);
            test_result[test_ind].fail_mark |= TEIMER_TEST_RESULT_START;
            goto _func_end;
        }

        if(hwtimer_step == 3)
        {
            wait_start = hal_fast_sys_timer_get();
            while(1)
            {
                wait_end = hal_fast_sys_timer_get();
                if(FAST_TICKS_TO_MS(wait_end - wait_start) >= TIMER_INTERVAL_MS*3)
                {
                    break;
                }
            }
            if(hwtimer_step == 3)
            {
                TRACE(1, "oneshot test done");
                break;
            }
        }
        if(hwtimer_step == 4)
        {
            test_result[test_ind].fail_mark |= TEIMER_TEST_RESULT_START;
            TRACE(1, "oneshot test fail! (not oneshot)");
            goto _func_end;
        }

    }

    // periodic test.
    TRACE(1,"\n");
    TRACE(1, "periodic test begin");
    hwtimer_step = 0;
    hwtimer_test[test_ind].setup_fp(HAL_TIMER_TYPE_PERIODIC, _periodic_timer_handler);
    wait_start = hal_fast_sys_timer_get();
    _start_timer = wait_start;
    ticks = _ms_to_ticks(hwtimer_test[test_ind].is_fast, TIMER_INTERVAL_MS);
    hwtimer_test[test_ind].start_fp(ticks);

    while(1)
    {
        wait_end = hal_fast_sys_timer_get();
        if(FAST_TICKS_TO_MS(wait_end - wait_start) >= TIME_WATI_TIMEOUT)
        {
            TRACE(1, "Periodic test wait timeout!!! Need: %d(ms), Wait: %d, Step: %d",
                   (TIMER_INTERVAL_MS*LOOP_NR*2),
                   FAST_TICKS_TO_MS(wait_end - wait_start),
                   hwtimer_step);
            test_result[test_ind].fail_mark |= TEIMER_TEST_RESULT_START;
            goto _func_end;
        }
        if(hwtimer_step == 2)
        {
            wait_start = hal_fast_sys_timer_get();
            while(1)
            {
                wait_end = hal_fast_sys_timer_get();
                if(FAST_TICKS_TO_MS(wait_end - wait_start) >= TIMER_INTERVAL_MS*2)
                {
                    break;
                }
            }
            if(hwtimer_step == 2)
            {
                TRACE(1, "periodic test done");
                break;
            }
        }
        if(hwtimer_step == 3)
        {
            TRACE(1, "periodic test fail!!! ");
            test_result[test_ind].fail_mark |= TEIMER_TEST_RESULT_STOP;
            goto _func_end;
        }
    }

    TRACE(1, "\n");

_func_end:
    if(test_result[test_ind].fail_mark == 0)
    {
        TRACE(1, "%s test done", hwtimer_test[id].timer_name);
    }
    else
    {
        TRACE(1, "%s test fail! fail mark = 0x%x", hwtimer_test[id].timer_name, test_result[test_ind].fail_mark);
    }
    TRACE(1, "\n\n");
}

#endif // HWTIMER_TEST_FUNCTION

#ifdef HWTIMER_TEST_ACCURACY

#define USE_INTERNAL_OUTPUT
#define TIMER_TEST_GPIO_IN_PIN  HAL_GPIO_PIN_P1_5
#define TIMER_TEST_GPIO_OUT_PIN  HAL_GPIO_PIN_P1_6
#define DEV_THRESHOLD_VALUE  1000
// Accuracy requirements (PPM)
#define TIMER_ACCURACY_PPM   5
#define TIMER_ACCURACY_REQ   (TIMER_ACCURACY_PPM*100)
#ifndef TIMER_TEST_PERIOD_US
#define TIMER_TEST_PERIOD_US  (100*1000)
#endif

#ifndef TIMER_TEST_MS
#define TIMER_TEST_MS  (1000*60*1) // 1 minute
#endif

typedef uint32_t (* get_sys_time_fp)(void);

typedef struct{
    bool is_fast;
    get_sys_time_fp get_time_fp;
}TIMER_ACCURACY;

typedef struct{
    int32_t left_dev_max;
    int32_t right_dev_max;
    int32_t left_dev_count;
    int32_t right_dev_count;
}TIMER_ACCURACY_RESULT;

TIMER_ACCURACY timer_accuracy[] =
{
{true, hal_fast_sys_timer_get},
{false, hal_sys_timer_get},
};

TIMER_ACCURACY_RESULT timer_accuracy_test_result[]=
{
{0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff},
{0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff},
};

static void _output_timer_handler(uint32_t elapsed)
{
    static uint8_t is_top = 0;
    static uint32_t index = 0;
    if(is_top == 1)
    {
        hal_gpio_pin_clr(TIMER_TEST_GPIO_OUT_PIN);
        is_top = 0;

    }
    else
    {
        hal_gpio_pin_set(TIMER_TEST_GPIO_OUT_PIN);
        is_top = 1;
    }
    if((index++) % 10 == 0)
    {
        //TRACE(1, "is_top = %d", is_top);
    }
}

static void _start_output(void)
{
#ifdef USE_INTERNAL_OUTPUT
    hal_gpio_pin_set_dir(TIMER_TEST_GPIO_OUT_PIN, HAL_GPIO_DIR_OUT,1);
    hal_fast_timer_setup(HAL_TIMER_TYPE_PERIODIC,_output_timer_handler);
    hal_fast_timer_start(_us_to_ticks(true,TIMER_TEST_PERIOD_US));
#endif
}

static void _stop_output(void)
{
#ifdef USE_INTERNAL_OUTPUT
    hal_fast_timer_stop();
#endif
}

static void _test_accuracy(TIMER_ACCURACY* accu, TIMER_ACCURACY_RESULT *result, uint32_t test_time_ms, uint32_t period_us)
{
    uint32_t start_time = hal_fast_sys_timer_get();
    uint32_t cur_time;
    uint32_t wait_time;
    int32_t wait_us;
    int32_t left_dev_max = 0;
    int32_t right_dev_max = 0;
    int32_t left_dev_count = 0;
    int32_t right_dev_count = 0;
    uint32_t count = 0;

    uint8_t val;
    uint8_t state = 1;

    TRACE(1, "%s: is_fast: %d, test_time_ms = %d, period_us = %08d", __func__, accu->is_fast, test_time_ms, period_us);
    hal_gpio_pin_set_dir(TIMER_TEST_GPIO_IN_PIN, HAL_GPIO_DIR_IN,1);

    start_time = accu->get_time_fp();
    while(1)
    {
        val = hal_gpio_pin_get_val(TIMER_TEST_GPIO_IN_PIN);
        if(state != val)
        {
            count ++;
            state = val;
            TRACE(1, "%s: val = %d", __func__, val);
        }
        if(count >= 20)
        {
           // state = 0;
            cur_time = accu->get_time_fp();
            wait_time = cur_time;

            break;
        }
    }

    while(1)
    {
        if(_ticks_to_us(accu->is_fast, cur_time - start_time) >= test_time_ms*1000)
        {
            break;
        }

        val = hal_gpio_pin_get_val(TIMER_TEST_GPIO_IN_PIN);
        if(state != val)
        {
            state = val;
            cur_time = accu->get_time_fp();
            wait_us = _ticks_to_us(accu->is_fast, cur_time - wait_time);
            wait_time = cur_time;
            if(period_us > wait_us)
            {
                left_dev_count ++ ;
                if(left_dev_max < period_us - wait_us)
                {
                    left_dev_max = period_us - wait_us;
                }
                if(period_us - wait_us > DEV_THRESHOLD_VALUE)
                {
                    TRACE(1, "is_fast: %d, LEFT: %08d", accu->is_fast, period_us - wait_us);
                }
            }
            else if(period_us < wait_us)
            {
                right_dev_count ++ ;
                if(right_dev_max < wait_us - period_us)
                {
                    right_dev_max = wait_us - period_us;
                }
                if(wait_us - wait_us > DEV_THRESHOLD_VALUE)
                {
                    TRACE(1, "is_fast: %d, RIGHT: %08d", accu->is_fast, wait_us - period_us);
                }
            }
            TRACE(1, "%s: val = %d", __func__, val);
            //TRACE(1, "%s: test_time = %08d---%08d", __func__, _ticks_to_us(accu->is_fast, cur_time - start_time),test_time_ms*1000);

        }
    }

    result->left_dev_count = left_dev_count;
    result->left_dev_max = left_dev_max;
    result->right_dev_count = right_dev_count;
    result->right_dev_max = right_dev_max;

    TRACE(1, "left_dev_count = %08d, left_dev_max = %08d", left_dev_count, left_dev_max);
    TRACE(1, "right_dev_count = %08d, right_dev_max = %08d", right_dev_count, right_dev_max);
    TRACE(1, "%s: done", __func__);

}

void _hwtimer_test_accuracy(void)
{
    uint32_t i;

    for(i = 0; i < sizeof(timer_accuracy)/sizeof(timer_accuracy[0]); i++)
    {
        _test_accuracy(&timer_accuracy[i], &timer_accuracy_test_result[i], TIMER_TEST_MS, TIMER_TEST_PERIOD_US);
    }
}

#endif

static void _print_test_result(void)
{
    uint32_t i = 0;

    i = i;
    TRACE(1, "\n\n");
    TRACE(1, "---------TIMER TEST RESULT----------");
#ifdef HWTIMER_TEST_FUNCTION
    TRACE(1, "---------FUNCTION----------");
    for(i = 0; i < sizeof(test_result)/sizeof(test_result[0]); i++)
    {
        if(test_result[i].fail_mark == 0)
        {
            TRACE(1, "%s TEST:  PASS", hwtimer_test[i].timer_name);
        }
        else
        {
            TRACE(1, "%s TEST:  NG", hwtimer_test[i].timer_name);
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_SETUP)
            {
                TRACE(1, "timer_setup FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_START)
            {
                TRACE(1, "timer_start FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_STOP)
            {
                TRACE(1, "timer_stop FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_CONTINUE)
            {
                TRACE(1, "timer_continue FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_CONTINUE)
            {
                TRACE(1, "timer_setup FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_IS_ENABLE)
            {
                TRACE(1, "timer_is_enable FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_RELOAD)
            {
                TRACE(1, "timer_reload FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_GET_RAW_VAL)
            {
                TRACE(1, "timer_get_raw_val FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_IRQ_ACTIVE)
            {
                TRACE(1, "timer_irq_active FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_IRQ_PENDING)
            {
                TRACE(1, "timer_irq_pending FAIL");
            }
            if(test_result[i].fail_mark & TEIMER_TEST_RESULT_GET_ELAPSED_TIME)
            {
                TRACE(1, "timer_get_elapsed FAIL");
            }
        }
    }
#endif // HWTIMER_TEST_FUNCTION

#ifdef HWTIMER_TEST_ACCURACY
    for(i = 0; i < sizeof(timer_accuracy_test_result)/sizeof(timer_accuracy_test_result[0]); i++)
    {
        if(timer_accuracy_test_result[i].left_dev_max < TIMER_ACCURACY_REQ &&
           timer_accuracy_test_result[i].right_dev_max < TIMER_ACCURACY_REQ)
        {
            if(timer_accuracy[i].is_fast)
            {
                TRACE(1,"FAST TIMER ACCURACY TEST:  PASS");
            }
            else
            {
                TRACE(1, "SLOW TIMER ACCURACY TEST:  PASS");
            }

        }
        else
        {
            if(timer_accuracy[i].is_fast)
            {
                TRACE(1, "FAST TIMER ACCURACY TEST:  NG");
            }
            else
            {
                TRACE(1, "SLOW TIMER ACCURACY TEST:  NG");
            }
        }
        TRACE(1, "LEFT DEV COUNT: %08d, MAX LEFT DEV VALUE: %08d",
            timer_accuracy_test_result[i].left_dev_count, timer_accuracy_test_result[i].left_dev_max);
        TRACE(1, "RIGHT DEV COUNT: %08d, MAX RIGHT DEV VALUE: %08d",
            timer_accuracy_test_result[i].right_dev_count, timer_accuracy_test_result[i].right_dev_max);

    }
#endif
    TRACE(1, "---------TIMER TEST RESULT----------");
    TRACE(1, "\n\n");
}

// output square wave
#ifdef OUTPUT_SQUARE_WAVE

static void _output_square_wave_do(TIMER_ACCURACY* accu, uint32_t period_us)
{
    uint32_t start_time;
    uint32_t cur_time;
    bool is_top = 1;
    int index = 0;

#ifdef CALIB_SLOW_TIMER_WITH_SLEEP
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
#endif

    TRACE(1, "%s: is_fast: %d, period_us = %d", __func__, accu->is_fast, period_us);
    start_time = accu->get_time_fp();
    while(1)
    {
        cur_time = accu->get_time_fp();
        if(_ticks_to_us(accu->is_fast, cur_time - start_time) < period_us)
        {
            continue;
        }
        start_time = cur_time;

        if(is_top == 1)
        {
            hal_gpio_pin_clr(TIMER_TEST_GPIO_OUT_PIN);
            is_top = 0;

        }
        else
        {
            hal_gpio_pin_set(TIMER_TEST_GPIO_OUT_PIN);
            is_top = 1;
        }
        if((index++) % 10 == 0)
        {
            //TRACE(1, "is_top = %d", is_top);
        }
#ifdef CALIB_SLOW_TIMER_WITH_SLEEP
        osDelay((period_us - 100)/1000);
#endif
    }

    TRACE(1, "%s: done", __func__);
}

static void _output_square_wave(void)
{
    int ind = 0;

    hal_gpio_pin_set_dir(TIMER_TEST_GPIO_OUT_PIN, HAL_GPIO_DIR_OUT,1);
#if TEST_TIMER_IND == 0
    ind = 0;
#elif  TEST_TIMER_IND == 1
    ind = 1;
#else
    #error test time ind
#endif
    TRACE(1, "OUTPUT SQUARE WAVE: TIMER: %d, PIN: %d PERIOD_US: %08d",
         ind,TIMER_TEST_GPIO_OUT_PIN, TIMER_TEST_PERIOD_US);

    _output_square_wave_do(&timer_accuracy[ind],
            TIMER_TEST_PERIOD_US);
}

#endif

#if defined(RTOS) && defined(FAST_TIMER_COMPENSATE) && defined(CALIB_SLOW_TIMER_CALIB_IN_THREAD)
static void slow_timer_calib_thread(const void *arg)
{
    int count = 0;
    while(1) {
        hal_sys_timer_calib();
        if (count < 10) {
            //calibrate slow time more often after power up, utils 32K stabilized
            osDelay(TIMER_CALIB_INTERVAL_TIME_1);
            count ++;
            TRACE(1, "SLOW TIMER CALIB IN THREAD, INTERVAL TIME: %08d", TIMER_CALIB_INTERVAL_TIME_1);
        } else {
            osDelay(TIMER_CALIB_INTERVAL_TIME_2);
            TRACE(1, "SLOW TIMER CALIB IN THREAD, INTERVAL TIME: %08d", TIMER_CALIB_INTERVAL_TIME_2);
        }
    }
}

osThreadDef(slow_timer_calib_thread, osPriorityBelowNormal, 1,
    1024, "slow_timer_calibration_thread");

static osThreadId _thread_id = NULL;

static void POSSIBLY_UNUSED init_slow_timer_calib_thread(void)
{
    if (_thread_id == NULL) {
        _thread_id = osThreadCreate(osThread(slow_timer_calib_thread), NULL);
    }
    TRACE(1, "SLOW TIMER THRAD CALIB START");
}
#endif

void function_test_hwtimer(void)
{
#if defined(RTOS) && defined(FAST_TIMER_COMPENSATE) && defined(CALIB_SLOW_TIMER_CALIB_IN_THREAD)
     init_slow_timer_calib_thread();
#endif

#ifdef OUTPUT_SQUARE_WAVE
    _output_square_wave();
#endif

#ifdef HWTIMER_TEST_FUNCTION
   _hwtimer_test_function(TIMER_TEST_TIMER0);
   _hwtimer_test_function(TIMER_TEST_TIMER1);
   _hwtimer_test_function(TIMER_TEST_TIMER2);
#endif

#ifdef HWTIMER_TEST_ACCURACY
    _start_output();
    _hwtimer_test_accuracy();
    _stop_output();
#endif
   _print_test_result();
   while(1);
}

#endif

#ifdef CHIP_FUNCTION_TEST_RTC
#include "string.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_timer_fast_irq.h"
#include "hal_timer_user_irq.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "pmu.h"
#include "hal_gpio.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_sysfreq.h"

#ifndef RTC_ENABLE
#err: RTC_ENABLE NOT OPENED!!!
#endif
static bool rtc_test_done = false;
#define BASE_SENCONDS  100
#define INTERVAL_TIME  5
#define TEST_TIMES     2
static void rtc_test_handler(uint32_t seconds)
{
    static uint32_t index = 0;
    TRACE(0, "%s: seconds: %d, index: %d ", __func__, (uint32_t)seconds , index);
    if(index++ < TEST_TIMES)
    {
        pmu_rtc_set_alarm((uint32_t)seconds + INTERVAL_TIME);
    }
    else
    {
        rtc_test_done = true;
    }
}

void function_test_rtc(void)
{
    TRACE(1, "RTC Test");
    TRACE(1, "RTC Test %d times,every RTC interval: %d(s)", TEST_TIMES, INTERVAL_TIME);
    pmu_rtc_enable();
    pmu_rtc_set(BASE_SENCONDS);
    pmu_rtc_set_alarm(BASE_SENCONDS + INTERVAL_TIME);
    pmu_rtc_set_irq_handler(rtc_test_handler);
    while(rtc_test_done == false)
    {
        osDelay(1);
    }
    TRACE(1, "RTC Test dnoe");
    TRACE(1, "\n\n");
    TRACE(1, "RTC TEST PASS");
    TRACE(1, "\n\n");
    //while(1);
}
#endif

