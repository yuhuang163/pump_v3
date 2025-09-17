/*
Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Original Author: Shay Gal-on
*/
#include "coremark.h"
#include "core_portme.h"
#include "cmsis.h"
#include "hal_sysfreq.h"
#include "plat_types.h"

#define TEST_FREQ                       MACRO_CONCAT(HAL_CMU_FREQ_, MACRO_CONCAT(TEST_FREQ_MHZ, M))

#if VALIDATION_RUN
volatile ee_s32 seed1_volatile = 0x3415;
volatile ee_s32 seed2_volatile = 0x3415;
volatile ee_s32 seed3_volatile = 0x66;
#endif
#if PERFORMANCE_RUN
volatile ee_s32 seed1_volatile = 0x0;
volatile ee_s32 seed2_volatile = 0x0;
volatile ee_s32 seed3_volatile = 0x66;
#endif
#if PROFILE_RUN
volatile ee_s32 seed1_volatile = 0x8;
volatile ee_s32 seed2_volatile = 0x8;
volatile ee_s32 seed3_volatile = 0x8;
#endif
volatile ee_s32 seed4_volatile = ITERATIONS;
volatile ee_s32 seed5_volatile = 0;
/* Porting : Timing functions
        How to capture time and convert to seconds must be ported to whatever is
   supported by the platform. e.g. Read value from on board RTC, read value from
   cpu clock cycles performance counter etc. Sample implementation for standard
   time.h and windows.h definitions included.
*/
CORETIMETYPE
barebones_clock()
{
    //#error "You must implement a method to measure time in barebones_clock()! This function should return current time. "
    uint32_t current_time = hal_fast_sys_timer_get();
    return current_time;
}
/* Define : TIMER_RES_DIVIDER
        Divider to trade off timer resolution and total time that can be
   measured.

        Use lower values to increase resolution, but make sure that overflow
   does not occur. If there are issues with the return value overflowing,
   increase this value.
        */
#define GETMYTIME(_t)              (*_t = barebones_clock())
#define MYTIMEDIFF(fin, ini)       ((fin) - (ini))
#define TIMER_RES_DIVIDER          1
#define SAMPLE_TIME_IMPLEMENTATION 1
#define CLOCKS_PER_SEC             (hal_cmu_get_fast_timer_freq())
#define EE_TICKS_PER_SEC           (CLOCKS_PER_SEC / TIMER_RES_DIVIDER)

/** Define Host specific (POSIX), or target specific global time variables. */
static CORETIMETYPE start_time_val, stop_time_val;
static uint32_t lock;

static void coremark_event_begin(void)
{
#ifdef SIMU
#ifdef __arm__
    __SEV();
#else
    hal_cmu_simu_set_val((get_cpu_id() << 16) | 1);
#endif
#endif
}

static void coremark_event_end(void)
{
#ifdef SIMU
#ifdef __arm__
    __SEV();
#else
    hal_cmu_simu_set_val((get_cpu_id() << 16) | 0);
#endif
#endif
}

/* Function : start_time
        This function will be called right before starting the timed portion of
   the benchmark.

        Implementation may be capturing a system timer (as implemented in the
   example code) or zeroing some system parameters - e.g. setting the cpu clocks
   cycles to 0.
*/
void
start_time(void)
{
    lock = int_lock();

    GETMYTIME(&start_time_val);
    coremark_event_begin();
}
/* Function : stop_time
        This function will be called right after ending the timed portion of the
   benchmark.

        Implementation may be capturing a system timer (as implemented in the
   example code) or other system parameters - e.g. reading the current value of
   cpu cycles counter.
*/
void
stop_time(void)
{
    coremark_event_end();
    GETMYTIME(&stop_time_val);
    int_unlock(lock);

    //ee_printf("start time %dms\n", FAST_TICKS_TO_MS(start_time_val));
    //ee_printf("stop time %dms\n", FAST_TICKS_TO_MS(stop_time_val));
}
/* Function : get_time
        Return an abstract "ticks" number that signifies time on the system.

        Actual value returned may be cpu cycles, milliseconds or any other
   value, as long as it can be converted to seconds by <time_in_secs>. This
   methodology is taken to accommodate any hardware or simulated platform. The
   sample implementation returns millisecs by default, and the resolution is
   controlled by <TIMER_RES_DIVIDER>
*/
CORE_TICKS
get_time(void)
{
    CORE_TICKS elapsed
        = (CORE_TICKS)(MYTIMEDIFF(stop_time_val, start_time_val));
    return elapsed;
}
/* Function : time_in_secs
        Convert the value returned by get_time to seconds.

        The <secs_ret> type is used to accommodate systems with no support for
   floating point. Default implementation implemented by the EE_TICKS_PER_SEC
   macro above.
*/
secs_ret
time_in_secs(CORE_TICKS ticks)
{
    secs_ret retval = ((secs_ret)ticks) / (secs_ret)EE_TICKS_PER_SEC;
#if HAS_FLOAT
    //ee_printf("%s, ticks %d, =%fms\n", __func__, ticks, retval/1000);
#else
    //ee_printf("%s, ticks %d, =%dms\n", __func__, ticks, retval/1000);
#endif
    return retval;
}

secs_ret
time_in_millisecs(CORE_TICKS ticks)
{
    secs_ret retval = ((secs_ret)ticks) / (secs_ret)(EE_TICKS_PER_SEC / 1000);
    return retval;
}

ee_u32 default_num_contexts = 1;

/* Function : portable_init
        Target specific initialization code
        Test for some common mistakes.
*/
void
portable_init(core_portable *p, int *argc, char *argv[])
{
    //#error "Call board initialization routines in portable init (if needed), in particular initialize UART! "
    TRACE_IMM(0, " ");
#ifdef TEST_FREQ
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, TEST_FREQ);
#endif
    TRACE_IMM(0, "CPU frequency changed to: %uM", (hal_sys_timer_calc_cpu_freq(5, 0) / 1000000));
    TRACE_IMM(0, "------coremark test started------");
    TRACE_IMM(0, "After at least 10 seconds, please wait...");
    ee_printf("\n");
    ee_printf("\n");

    if (sizeof(ee_ptr_int) != sizeof(ee_u8 *))
    {
        ee_printf(
            "ERROR! Please define ee_ptr_int to a type that holds a "
            "pointer!\n");
    }
    if (sizeof(ee_u32) != 4)
    {
        ee_printf("ERROR! Please define ee_u32 to a 32b unsigned type!\n");
    }
    p->portable_id = 1;
}
/* Function : portable_fini
        Target specific final code
*/
void
portable_fini(core_portable *p)
{
    p->portable_id = 0;
}
