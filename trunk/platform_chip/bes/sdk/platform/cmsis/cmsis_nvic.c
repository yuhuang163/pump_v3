/* mbed Microcontroller Library
 * CMSIS-style functionality to support dynamic vectors
 *******************************************************************************
 * Copyright (c) 2011 ARM Limited. All rights reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of ARM Limited nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 */
#ifndef __ARM_ARCH_ISA_ARM

#include "plat_types.h"
#include "plat_addr_map.h"
#include "cmsis_nvic.h"
#include "hal_location.h"
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif
#include "mpu.h"
#if defined(CHIP_BEST1501SIMU) || defined(CHIP_BEST1600SIMU)
#include "hal_cmu.h"
#endif

STATIC_ASSERT(NVIC_NUM_VECTORS * 4 <= VECTOR_SECTION_SIZE, "ERROR: VECTOR_SECTION_SIZE too small!");

// The vector table must be aligned to NVIC_NUM_VECTORS-word boundary, rounding up to the next power of two
// -- 0x100 for 33~64 vectors, and 0x200 for 65~128 vectors
#ifdef __ARMCC_VERSION
#define VECTOR_LOC                              __attribute__((section(".bss.vector_table"))) USED
#else
#define VECTOR_LOC                              __attribute__((section(".vector_table"))) USED
#endif

#define FAULT_HANDLER                           __attribute__((weak,alias("NVIC_default_handler")))

static uint32_t VECTOR_LOC vector_table[NVIC_NUM_VECTORS];

__STATIC_FORCEINLINE void __NVIC_DisableAllIRQs(void)
{
    int i;

    for (i = 0; i < (USER_IRQn_QTY + 31) / 32; i++) {
        NVIC->ICER[i] = ~0UL;
    }
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    SCB->VTOR = ROMX_BASE;
}

BOOT_TEXT_FLASH_LOC void NVIC_DisableAllIRQs(void)
{
    __NVIC_DisableAllIRQs();
}

BOOT_TEXT_SRAM_LOC void NVIC_DisableAllIRQs_Ram(void)
{
    __NVIC_DisableAllIRQs();
}

static void NAKED BOOT_TEXT_FLASH_LOC NVIC_default_handler(void)
{
#if defined(CHIP_BEST1501SIMU) || defined(CHIP_BEST1600SIMU)
    hal_cmu_simu_set_val(0xeeeeeeee);
#endif
    asm volatile(
                "1:\n"
                "nop\n"
                "nop\n"
                "nop\n"
                "nop\n"
                "b 1b\n");
}

void FAULT_HANDLER Reset_Handler(void);
void FAULT_HANDLER NMI_Handler(void);
void FAULT_HANDLER HardFault_Handler(void);
void FAULT_HANDLER MemManage_Handler(void);
void FAULT_HANDLER BusFault_Handler(void);
void FAULT_HANDLER UsageFault_Handler(void);
void FAULT_HANDLER SecureFault_Handler(void);
void FAULT_HANDLER SVC_Handler(void);
void FAULT_HANDLER DebugMon_Handler(void);
void FAULT_HANDLER PendSV_Handler(void);
void FAULT_HANDLER SysTick_Handler(void);
extern uint32_t __rom_stack[];
extern uint32_t __stack[];

static const uint32_t BOOT_RODATA_FLASH_LOC fault_handlers[NVIC_USER_IRQ_OFFSET] = {
#if defined(ROM_BUILD) && !defined(ROM_IN_FLASH)
    (uint32_t)__rom_stack,
#else
    (uint32_t)__stack,
#endif
    (uint32_t)Reset_Handler,
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    (uint32_t)SecureFault_Handler,
    (uint32_t)NVIC_default_handler,
    (uint32_t)NVIC_default_handler,
    (uint32_t)NVIC_default_handler,
    (uint32_t)SVC_Handler,
    (uint32_t)DebugMon_Handler,
    (uint32_t)NVIC_default_handler,
    (uint32_t)PendSV_Handler,
    (uint32_t)SysTick_Handler,
};

#ifdef COMMON_IRQ_ENTRY
uint32_t BOOT_BSS_LOC os_vector_table[NVIC_NUM_VECTORS];
NVIC_IRQ_ENTRY_CB_T BOOT_BSS_LOC os_irq_entry_cb;
#ifndef NO_IRQ_EXIT_CB
NVIC_IRQ_EXIT_CB_T BOOT_BSS_LOC os_irq_exit_cb;
#endif

void NVIC_RegisterIrqEntryCallback(NVIC_IRQ_ENTRY_CB_T cb)
{
    os_irq_entry_cb = cb;
}

void NVIC_RegisterIrqExitCallback(NVIC_IRQ_EXIT_CB_T cb)
{
#ifndef NO_IRQ_EXIT_CB
    os_irq_exit_cb = cb;
#endif
}

BOOT_TEXT_SRAM_LOC NAKED
void vector_common_entry(void)
{
    asm volatile (
        "mrs r0, ipsr;"
#if 0
        "ldr r1, =" TO_STRING(IPSR_ISR_Msk) ";"
        "ands r0, r1;"
#endif
        "ldr r2, =os_irq_entry_cb;"
        "ldr r3, [r2];"
#ifdef NO_IRQ_EXIT_CB
        "cbz r3, _irq_handler_wo_exit_cb;"
        "push {r0, lr};"
#else
        "push {r0, lr};"
        "cbz r3, _irq_handler_w_exit_cb;"
#endif
        "subs r0, #" TO_STRING(NVIC_USER_IRQ_OFFSET) ";"
        "blx r3;"

#ifndef NO_IRQ_EXIT_CB
        "mrs r0, ipsr;"
        "_irq_handler_w_exit_cb:;"
        "subs r1, r0, #" TO_STRING(NVIC_USER_IRQ_OFFSET) ";"
        // Skip irq exit callback for internal interrupts (OS handling needs the original stack)
        "blt _skip_irq_exit_cb;"
        "ldr r1, =os_vector_table;"
        "ldr r2, [r1, r0, LSL #2];"
        "blx r2;"
        "mrs r0, ipsr;"
        "subs r0, #" TO_STRING(NVIC_USER_IRQ_OFFSET) ";"
        "ldr r2, =os_irq_exit_cb;"
        "ldr r3, [r2];"
        "cbz r3, _exit_common_entry;"
        "blx r3;"
        "_exit_common_entry:;"
        // NOTE: Ensure to restore the original stack when
        // page_spy_irq_handler() falls back to MemManage_Handler()
        "pop {r0, lr};"
        "bx lr;"
#endif

        "_skip_irq_exit_cb:;"
        "pop {r0, lr};"
        "_irq_handler_wo_exit_cb:;"
        "ldr r1, =os_vector_table;"
        "ldr r2, [r1, r0, LSL #2];"
        "bx r2;"

#ifdef LTO_OPTIMIZE
        ".pool;"
        : : "X"(os_irq_entry_cb),
#ifndef NO_IRQ_EXIT_CB
            "X"(os_irq_exit_cb),
#endif
            "X"(os_vector_table)
#endif
    );
}

void BOOT_TEXT_FLASH_LOC NVIC_InitCommonIrqEntry(void)
{
    int i;

    for (i = 2; i < NVIC_NUM_VECTORS; i++) {
        os_vector_table[i] = vector_table[i];
        vector_table[i] = (uint32_t)vector_common_entry;
    }
}
#endif

void BOOT_TEXT_FLASH_LOC NVIC_InitVectors(void)
{
    int i;

    for (i = 0; i < NVIC_NUM_VECTORS; i++) {
        vector_table[i] = (i < ARRAY_SIZE(fault_handlers)) ?
            fault_handlers[i] : (uint32_t)NVIC_default_handler;
    }

    SCB->VTOR = (uint32_t)vector_table;
    __DSB();
}

void NVIC_SetNMIHandler(NVIC_DEFAULT_FAULT_HANDLER_T handler)
{
#ifdef COMMON_IRQ_ENTRY
    os_vector_table[2] = (uint32_t)handler;
#else
    vector_table[2] = (uint32_t)handler;
#endif
}

void NVIC_SetDefaultFaultHandler(NVIC_DEFAULT_FAULT_HANDLER_T handler)
{
    int i;

    for (i = 1; i < ARRAY_SIZE(fault_handlers); i++) {
        if (vector_table[i] == (uint32_t)NVIC_default_handler) {
            vector_table[i] = (uint32_t)handler;
        }
#ifdef COMMON_IRQ_ENTRY
        if (i >= 2 && os_vector_table[i] == (uint32_t)NVIC_default_handler) {
            os_vector_table[i] = (uint32_t)handler;
        }
#endif
    }
}

void BOOT_TEXT_FLASH_LOC NVIC_SetResetHandler(NVIC_DEFAULT_FAULT_HANDLER_T handler)
{
    vector_table[1] = (uint32_t)handler;
}

void NVIC_SetIrqHandler(uint16_t irq, NVIC_DEFAULT_FAULT_HANDLER_T handler)
{
#ifdef COMMON_IRQ_ENTRY
    if (irq >= 2 && vector_table[irq] == (uint32_t)vector_common_entry) {
        os_vector_table[irq] = (uint32_t)handler;
        return;
    }
#endif
    vector_table[irq] = (uint32_t)handler;
}

uint32_t* NVIC_GetVectorTab()
{
    return vector_table;
}

IRQn_Type NVIC_GetCurrentActiveIRQ(void)
{
    IRQn_Type irq = (__get_IPSR() & IPSR_ISR_Msk) - NVIC_USER_IRQ_OFFSET;
    return irq;
}

#ifdef CORE_SLEEP_POWER_DOWN

void SRAM_TEXT_LOC NVIC_PowerDownSleep(uint32_t *buf, uint32_t cnt)
{
    int i;
    uint32_t idx;
    __IO uint32_t *regs;

    mpu_sleep();

    idx = 0;

#ifdef ARM_CMSE
    uint32_t sau_num;

    sau_num = (SAU->TYPE & SAU_TYPE_SREGION_Msk) >> SAU_TYPE_SREGION_Pos;
    for (i = 0; i < sau_num; i++) {
        SAU->RNR = (i & SAU_RNR_REGION_Msk);
        buf[idx++] = SAU->RBAR;
        buf[idx++] = SAU->RLAR;
    }

    buf[idx++] = SAU->CTRL;

    for (i = 0; i < (NVIC_NUM_VECTORS - NVIC_USER_IRQ_OFFSET + 31) / 32; i++) {
        buf[idx++] = NVIC->ITNS[i];
    }
#endif
    for (i = 0; i < (NVIC_NUM_VECTORS - NVIC_USER_IRQ_OFFSET + 31) / 32; i++) {
        buf[idx++] = NVIC->ISER[i];
    }
#if (__CORTEX_M <= 4)
    regs = (__IO uint32_t *)&NVIC->IP[0];
#else
    regs = (__IO uint32_t *)&NVIC->IPR[0];
#endif
    for (i = 0; i < (NVIC_NUM_VECTORS - NVIC_USER_IRQ_OFFSET + 3) / 4; i++) {
        buf[idx++] = regs[i];
    }
    buf[idx++] = SCnSCB->ACTLR;
    buf[idx++] = SCB->ICSR;
    buf[idx++] = SCB->AIRCR;
    buf[idx++] = SCB->SCR;
    buf[idx++] = SCB->CCR;
#if (__CORTEX_M <= 4)
    regs = (__IO uint32_t *)&SCB->SHP[0];
#else
    regs = (__IO uint32_t *)&SCB->SHPR[0];
#endif
    buf[idx++] = regs[0];
    buf[idx++] = regs[1];
    buf[idx++] = regs[2];
    buf[idx++] = SCB->SHCSR;
    buf[idx++] = SCB->CPACR;
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
#ifdef ARM_CMSE
    buf[idx++] = SCB->NSACR;
#endif
    // Clear LSPACT (Lazy state)
    FPU->FPCCR &= ~FPU_FPCCR_LSPACT_Msk;
    buf[idx++] = FPU->FPCCR;
#endif

#if defined (__PMU_PRESENT) && (__PMU_PRESENT == 1U)
    // TODO: Save PMU/DWT registers
#endif

#if (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
    buf[idx++] = *(volatile uint32_t *)PFCR;
#endif

    buf[idx++] = SysTick->CTRL;
    buf[idx++] = SysTick->LOAD;

    if (idx > cnt) {
        do { asm volatile("nop \n nop \n nop \n nop"); } while (1);
    }
}

void SRAM_TEXT_LOC NVIC_PowerDownWakeup(uint32_t *buf, uint32_t cnt)
{
    int i;
    uint32_t idx;
    __IO uint32_t *regs;
    uint32_t systick_ctrl;

    SCB->VTOR = (uint32_t)vector_table;

    idx = 0;

#ifdef ARM_CMSE
    uint32_t sau_num;

    sau_num = (SAU->TYPE & SAU_TYPE_SREGION_Msk) >> SAU_TYPE_SREGION_Pos;
    for (i = 0; i < sau_num; i++) {
        SAU->RNR = (i & SAU_RNR_REGION_Msk);
        SAU->RBAR = buf[idx++];
        SAU->RLAR = buf[idx++];
    }
    __DMB();
    SAU->CTRL = buf[idx++];
    __DSB();
    __ISB();

    for (i = 0; i < (NVIC_NUM_VECTORS - NVIC_USER_IRQ_OFFSET + 31) / 32; i++) {
        NVIC->ITNS[i] = buf[idx++];
    }
#endif
    for (i = 0; i < (NVIC_NUM_VECTORS - NVIC_USER_IRQ_OFFSET + 31) / 32; i++) {
        NVIC->ISER[i] = buf[idx++];
    }
#if (__CORTEX_M <= 4)
    regs = (__IO uint32_t *)&NVIC->IP[0];
#else
    regs = (__IO uint32_t *)&NVIC->IPR[0];
#endif
    for (i = 0; i < (NVIC_NUM_VECTORS - NVIC_USER_IRQ_OFFSET + 3) / 4; i++) {
        regs[i] = buf[idx++];
    }
    SCnSCB->ACTLR = buf[idx++];
    SCB->ICSR = buf[idx++];
    SCB->AIRCR = buf[idx++];
    SCB->SCR = buf[idx++];
#if (__CORTEX_M <= 4)
    SCB->CCR = buf[idx++];
    regs = (__IO uint32_t *)&SCB->SHP[0];
#else
    SCB->CCR = buf[idx++] | (SCB->CCR & (SCB_CCR_IC_Msk | SCB_CCR_DC_Msk));
    regs = (__IO uint32_t *)&SCB->SHPR[0];
#endif
    regs[0] = buf[idx++];
    regs[1] = buf[idx++];
    regs[2] = buf[idx++];
    SCB->SHCSR = buf[idx++];
    SCB->CPACR = buf[idx++];
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
#ifdef ARM_CMSE
    SCB->NSACR = buf[idx++];
#endif
    FPU->FPCCR = buf[idx++];
#endif

#if defined (__PMU_PRESENT) && (__PMU_PRESENT == 1U)
    // TODO: Restore PMU/DWT registers
    PMU->CTRL = 0;
#endif

#if (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
    *(volatile uint32_t *)PFCR = buf[idx++];
#endif

    systick_ctrl = buf[idx++];
    SysTick->CTRL = (systick_ctrl & ~SysTick_CTRL_ENABLE_Msk);
    SysTick->LOAD = buf[idx++];
    SysTick->VAL = 0;
    SysTick->CTRL = systick_ctrl;

    mpu_wakeup();
}

#endif

#endif

