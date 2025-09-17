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
#include "eshell.h"
#include "app_eshell.h"
#include "pmu.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"

#if defined(KERNEL_RHINO)
#include "debug_cpuusage.h"
#endif

#if defined(osCMSIS_FreeRTOS) || defined(KERNEL_LITEOS_M)

extern uint32_t __flashx_text_start__[];
extern uint32_t __flashx_text_end__[];
extern uint32_t __boot_text_sram_start__[];
extern uint32_t __boot_text_sram_end__[];
extern uint32_t __sram_text_start__[];
extern uint32_t __sram_text_end__[];
extern uint32_t __text_start__[];
extern uint32_t __text_end__[];
extern uint32_t __fast_sram_text_exec_start__[];
extern uint32_t __fast_sram_text_exec_end__[];
void print_possible_backtrace(uint32_t *sp,uint32_t *end)
{
    uint32_t lock = int_lock_global();
    uint32_t *stackpoint = sp;
    printf(">>>>>> backtrace: sp:%p end:%p\r\n",sp,end);

    while (1)
    {
        uint32_t *pc = (uint32_t *)*((uint32_t *)stackpoint);
        stackpoint++;
        if(stackpoint > end)
        {
            break;
        }

        if (((uint32_t)pc & 0x1) != 1 )
        {
            continue;
        }

        pc = (uint32_t *)((uint32_t)pc - 1);
#ifdef CHIP_BEST2003
        if (!(((pc >= __flashx_text_start__) && (pc < __flashx_text_end__)) ||
            ((pc >= __boot_text_sram_start__) && (pc < __boot_text_sram_end__)) ||
            ((pc >= __sram_text_start__) && (pc < __sram_text_end__)) ||
            ((pc >= __text_start__) && (pc < __text_end__)) ||
            ((pc >= __fast_sram_text_exec_start__) && (pc < __fast_sram_text_exec_end__))))
        {
            continue;
        }
#endif
        printf("bt: %p=%p\r\n",stackpoint-1,pc);
    }
    int_unlock_global(lock);
}

static void show_backtrace(int argc, char *argv[])
{
    if(argc < 1)
    {
        printf("ERROR input addr of stack\r\n");
        return;
    }

    char num_begin[9] = {0};
    char *ptr_begin = (char *)argv[1];

    if ( (*ptr_begin == '0') && (*(ptr_begin+1) == 'x' || *(ptr_begin+1) == 'X'))
    {
        for (size_t i = 0; i < 8; i++)
        {
            num_begin[i] = *(ptr_begin + 2 + i);
        }
    }

    uint32_t *sp = (uint32_t *)strtol (num_begin, NULL, 16);
    // uint32_t *end = (uint32_t *)strtol (num_end, NULL, 16);
     uint32_t *end = sp + 512;
    if(sp == NULL)
    {
        printf("sp is null\r\n");
        return;
    }
    printf("\r\n sp:%p end:%p \r\n",sp,end);
    print_possible_backtrace(sp,end);
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "backtrace", "show backtrace", show_backtrace);
#endif

#if defined(osCMSIS_FreeRTOS)  && ! defined(MODULE_KERNEL_STUB)
#include "FreeRTOS.h"
#include "task.h"
#define tskRUNNING_CHAR     ( 'X' )
#define tskBLOCKED_CHAR     ( 'B' )
#define tskREADY_CHAR       ( 'R' )
#define tskDELETED_CHAR     ( 'D' )
#define tskSUSPENDED_CHAR   ( 'S' )
void show_freertos_runtime_stat(void)
{
    uint8_t x;
    uint32_t ulRunTimeTick,uxArraySize;
    uxArraySize = uxTaskGetNumberOfTasks();
    TaskStatus_t * pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );
    if( pxTaskStatusArray != NULL )
    {
        /* Generate the (binary) data. */
        uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulRunTimeTick );
        printf("%15s %10s %10s %10s %10s %10s %10s %10s %10s\r\n"
            ,"TskName","TskNum","TskState","TskPrio","StackBase","StackPoint","TcbAddr","MinFreeStk","CPU Usage");
        char cStatus;
        for(x = 0;x < uxArraySize; x++)
        {
            switch( pxTaskStatusArray[ x ].eCurrentState )
            {
                case eRunning:      cStatus = tskRUNNING_CHAR;
                                    break;
                case eReady:        cStatus = tskREADY_CHAR;
                                    break;
                case eBlocked:      cStatus = tskBLOCKED_CHAR;
                                    break;
                case eSuspended:    cStatus = tskSUSPENDED_CHAR;
                                    break;
                case eDeleted:      cStatus = tskDELETED_CHAR;
                                    break;
                case eInvalid:      /* Fall through. */
                default:            cStatus = ( char ) 0x00;
                                    break;
            }
            float percent = (float)(pxTaskStatusArray[x].ulRunTimeCounter*1.0f * 100/ulRunTimeTick);
            printf("%15s %10d %10c %10d 0x%2x 0x%2x 0x%2x %2d %10d%%\r\n",
                    pxTaskStatusArray[x].pcTaskName,
                    (int)pxTaskStatusArray[x].xTaskNumber,
                    cStatus,
                    (int)pxTaskStatusArray[x].uxCurrentPriority,
                    ( unsigned int )pxTaskStatusArray[x].pxStackBase,
                    ( unsigned int )pxTaskStatusArray[x].pxStackPoint,
                    ( unsigned int )pxTaskStatusArray[x].xHandle,
                    (int)pxTaskStatusArray[x].usStackHighWaterMark,
                     (int)percent);
        }
        printf("freeHeapSize:%u, minimumEver:%u\r\n", xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize());
        printf("dump task ok!\r\n");
    }
    vPortFree( pxTaskStatusArray );
}
#endif

static void show_Runtime_Stat(int argc, char *argv[])
{
#if defined(osCMSIS_FreeRTOS) && !defined(MODULE_KERNEL_STUB)
    show_freertos_runtime_stat();
#elif defined(KERNEL_RHINO)
    debug_task_cpu_usage_stats();
    int cpu0 = debug_total_cpu_usage_get(0)/100;
    eshell_putstring_nl("cpu0=%d%%, freq:%dMhz", cpu0, hal_sys_timer_calc_cpu_freq(5, 0)/1000000);
    debug_total_cpu_usage_show(NULL, 0, 0);
#else
#endif

#ifdef KERNEL_LITEOS_M
    extern uint32_t OsGetAllTskInfo(void);
    OsGetAllTskInfo();
#endif

    return;
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "runtime", "show run time statistics", show_Runtime_Stat);


#if 0
static void show_free_heap_size(int argc, char *argv[])
{
    extern uint32_t LOS_MemPoolList(void);
    LOS_MemPoolList();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "freeheapsize", "show RTOS/application free heap size", show_free_heap_size);

extern void nts_set_shoottsk(const char *name);
static void set_shoottsk(int argc, char *argv[])
{
    if (argc != 2) {
        eshell_putstring("agrc:%d\n", argc);
        return;
    }
    nts_set_shoottsk(argv[1]);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "shoottsk", "trace dedicate thread heap malloc-free", set_shoottsk);


static void show_system_info(int argc, char *argv[])
{
    eshell_putstring("========== Heap Info  ==========\r\n");
    show_free_heap_size(argc, argv);

    eshell_putstring("========== Task Info  =========\r\n");
    show_Runtime_Stat(argc, argv);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "debugsys", "show system info [heap,task]", show_system_info);
#endif

static void panic_trigger(int argc, char *argv[])
{
    __asm__ __volatile__("udf" ::: "memory");
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "panic", "start panic", panic_trigger);

#ifdef __SYS_AS_MAIN__
static void pmu_read_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  pmu_read <addr-hexformat>\r\n");
    eshell_putstring("  <addr:0x0000-0xF000>\r\n");
}
// pmu_read 00
static void pmu_read_reg(int argc, char *argv[])
{
    uint32_t addr;
    unsigned short reg_val;
    if (argc != 2) {
        goto usage;
    }
    addr = strtoul(argv[1], NULL, 16);

    if (addr > 0xF000) {
        goto usage;
    }

    pmu_read((unsigned short)addr, &reg_val);
    eshell_putstring("pmu reg:0x%04x = 0x%04x\n", addr, reg_val);
    return;
usage:
    pmu_read_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "pmu_read", "pmu read", pmu_read_reg);

static void pmu_write_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  pmu_write <addr-hexformat> <data-hexformat>\r\n");
    eshell_putstring("  <addr:0x0000-0xF000> <data:0x0000-0xFFFF>\r\n");
}
// pmu_write 00,1861
static void pmu_write_reg(int argc, char *argv[])
{
    uint32_t addr;
    uint32_t val;
    unsigned short reg_val;
    if (argc != 3) {
        goto usage;
    }
    addr = strtoul(argv[1], NULL, 16);
    val = strtoul(argv[2], NULL, 16);

    if (addr > 0xF000) {
        goto usage;
    }

    reg_val = val;
    pmu_write((unsigned short)addr, reg_val);
    eshell_putstring("pmu write reg:0x%04x=0x%04x done .\n", addr, reg_val);
    return;
usage:
    pmu_write_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "pmu_write", "pmu write", pmu_write_reg);
// todo
/*
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "mipiphy_write", "mipiphy write", mipiphy_write_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "mipiphy_read", "mipiphy read", mipiphy_read_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "psramphy0_write", "psramphy0 write", psramphy0_write_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "psramphy0_read", "psramphy0 read", psramphy0_read_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "psramphy1_write", "psramphy1 write", psramphy1_write_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "psramphy1_read", "psramphy1 read", psramphy1_read_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "usbphy_write", "usbphy write", usbphy_write_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "usbphy_read", "usbphy read", usbphy_read_reg);
*/
#endif //__SYS_AS_MAIN__

static bool debug = false;
static bool do_restart_eshell = false;
static osThreadId_t restart_eshell_thread_id = NULL;

static void restart_eshell_thread(void const *notused)
{
    while(1)
    {
        if(do_restart_eshell){
            eshell_close();
            // eshell_close_uart_rx();
            if(debug)
                eshell_open(HAL_UART_ID_1, true);
            else
                eshell_open(HAL_UART_ID_0, true);

            do_restart_eshell = false;
        }
        osDelay(100);
    }
}

static void enable_debug_i2c(bool temp_debug)
{
    if(!do_restart_eshell){
        if(temp_debug){
            hal_trace_close();
            hal_iomux_set_analog_i2c();
            hal_iomux_set_uart1();
            hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
            do_restart_eshell = true;
        }else{
            hal_trace_close();
            hal_iomux_set_uart0();
            hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
            do_restart_eshell = true;
        }
    }
}
#define ESHELL_DEBUG_TASK_STACK_SZ ROUND_UP(1024, 8)
static uint64_t os_thread_def_stack_restart_eshell_thread
                [ESHELL_DEBUG_TASK_STACK_SZ / sizeof(uint64_t)];
static const osThreadAttr_t ThreadAttr_Eshell_Debug = {
    .name = "restart_eshell_thread",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = os_thread_def_stack_restart_eshell_thread,
    .stack_size = ESHELL_DEBUG_TASK_STACK_SZ,
    .priority = osPriorityNormal,
    .tz_module = 1U,                  // indicate calls to secure mode
    .reserved = 0U,
};
static void restart_eshell_init(void)
{
    if (restart_eshell_thread_id)
        return;
    restart_eshell_thread_id = osThreadNew((osThreadFunc_t)restart_eshell_thread,
                                            NULL, &ThreadAttr_Eshell_Debug);
    do_restart_eshell = false;
}

static void restart_eshell_exit(void)
{
    if(restart_eshell_thread_id){
        osThreadTerminate(restart_eshell_thread_id);
        restart_eshell_thread_id = NULL;
    }
}

static void debug_i2c_cmd_usage(void)
{
    eshell_putstring("Usage: \r\n");
    eshell_putstring("  debug_i2c init, creat re-open eshell thread\r\n");
    eshell_putstring("  debug_i2c enable|disable, open debug_i2c\r\n");
    eshell_putstring("  debug_i2c exit, exit re-open eshell thread\r\n");
}

static void do_debug_i2c_set(int argc, char *argv[])
{
    if(argc < 2)
        goto usage;

    if(strncmp(argv[1], "init", 4) == 0){
        restart_eshell_init();
    }else if(strncmp(argv[1], "enable", 6) == 0){
        debug = true;
        enable_debug_i2c(debug);
    }else if(strncmp(argv[1], "disable", 7) == 0){
        debug = false;
        enable_debug_i2c(debug);
    }else if(strncmp(argv[1], "exit", 4) == 0)
        restart_eshell_exit();
    else
        goto usage;

    return;

usage:
    debug_i2c_cmd_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "debug_i2c", "debug_i2c help", do_debug_i2c_set);

#endif //UTILS_ESHELL_EN
