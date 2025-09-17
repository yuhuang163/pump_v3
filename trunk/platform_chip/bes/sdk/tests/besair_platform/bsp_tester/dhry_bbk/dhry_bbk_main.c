/*
 ****************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *                                                                            
 *  Version:    C, Version 2.1
 *                                                                            
 *  File:       dhry_1.c (part 2 of 3)
 *
 *  Date:       May 17, 1988
 *
 *  Author:     Reinhold P. Weicker
 *
 ****************************************************************************
 */
#include "dhry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hal_sysfreq.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_trace.h"


  /* variables for time measurement: */
  
  /* current time in milliseconds */
#define MSEC_PER_SEC            (1000LL)
#define NSEC_PER_MSEC           (1000000LL)
  
  uint32_t curr_time_ms() {
      uint32_t now_time;
      //now_time = hal_sys_timer_get();
      now_time = hal_fast_sys_timer_get();
      //return FAST_TICKS_TO_MS(now_time);
      return now_time;
  }
  uint32_t            Begin_Time,
                  End_Time,
                  User_Time;
  float           Microseconds,
                  Dhrystones_Per_Second;
  
  /* end of variables for time measurement */

void bbk_dhry_test()
{
    int  Number_Of_Runs = 1000000;

    while(1) {
        TRACE_IMM(0, "CPU frequency changed to: %u MHz", (hal_sys_timer_calc_cpu_freq(5, 0) / 1000000));
        TRACE_IMM(0, "------dhrystone test started------");
        TRACE_IMM(0,"dhrystone -- %d--start.\n",Number_Of_Runs);
        dhry_init();
        Begin_Time = (uint32_t) curr_time_ms();
        dhry_main(Number_Of_Runs);
        End_Time = (uint32_t)curr_time_ms();
        TRACE_IMM(0,"dhrystone-Begin_Time -- %d.\n", Begin_Time);
        TRACE_IMM(0,"dhrystone-End_Time -- %d.\n", End_Time);
        User_Time = FAST_TICKS_TO_MS(End_Time - Begin_Time);

        if (User_Time < 1000)
        {
            TRACE_IMM(0,"Measured time too small to obtain meaningful results\n");
            TRACE_IMM(0,"Please increase number of runs\n");
            TRACE_IMM(0,"\n");
        }
        else
        {
            Dhrystones_Per_Second = ((float) 1000 * (float) Number_Of_Runs)/ (float) User_Time;
            Dhrystones_Per_Second = (float)Dhrystones_Per_Second / (float)1757.0;
            TRACE_IMM(0,"User_Time '%d', dmips = %6.1f \n", User_Time, Dhrystones_Per_Second);
        }
        dhry_deinit();
        osDelay(1000);
    }

}

